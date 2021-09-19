#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>

#include "qdebug.h"

#include "completer.h"
#include "editor.h"
#include "line_number_area.h"
#include "mode.h"
#include "references_widget.h"
#include "syntax_highlighter.h"
#include "tasks.h"
#include "window.h"

Editor::Editor(Window* window) :
    tasksPlugin(nullptr),
    QPlainTextEdit(window),
    currentCompleter(nullptr),
    window(window),
    buffer(nullptr),
    mode(MODE_NORMAL),
    tabIndex(-1),
    highlightedLine(QColor::fromRgb(50, 50, 50)) {
    Q_ASSERT(window != nullptr);

    // line number area
    // ----------------------

    this->lineNumberArea = new LineNumberArea(this);
    this->onUpdateLineNumberAreaWidth(0);

    // editor font
    // ----------------------

    this->setFont(this->getFont());

    // basic theming
    // ----------------------

    this->setStyleSheet("color: #e7e7e7; background-color: #262626;");
    QPalette p = this->palette();
    p.setColor(QPalette::Highlight, QColor::fromRgb(70, 70, 70));
    p.setColor(QPalette::HighlightedText, QColor::fromRgb(240, 240, 240));
    this->setPalette(p);

    // selection timer
    // ----------------------

    this->selectionTimer = new QTimer;
    this->lspRefreshTimer = new QTimer;

    // tab space size
    // ----------------------

    const int tabSpace = 4;
    QFontMetrics metrics(this->getFont());
    this->setTabStopDistance(tabSpace*metrics.horizontalAdvance(" "));

    // 80 chars separator
    // ----------------------

    QString chars;
    for (int i = 0; i < 80; i++) { chars += "#"; }
    this->eightyCharsX = metrics.horizontalAdvance(chars);
    for (int i = 0; i < 40; i++) { chars += "#"; }
    this->hundredTwentyCharsX = metrics.horizontalAdvance(chars);

    // enabled plugins
    // ---------------

    this->tasksPlugin = new TasksPlugin(window);

    // start in normal mode
    // ----------------------

    this->setSubMode(NO_SUBMODE);
    this->setMode(MODE_NORMAL);

    connect(this, &QPlainTextEdit::selectionChanged, this, &Editor::onSelectionChanged);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &Editor::onCursorPositionChanged);
    connect(this->selectionTimer, &QTimer::timeout, this, &Editor::onTriggerSelectionHighlight);
    connect(this->lspRefreshTimer, &QTimer::timeout, this, &Editor::onTriggerLspRefresh);
    // line area
    connect(this, &QPlainTextEdit::blockCountChanged, this, &Editor::onUpdateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &Editor::onUpdateLineNumberArea);
}

Editor::~Editor() {
    if (this->buffer != nullptr) {
        this->buffer->onLeave(); // store settings
        this->buffer->onClose();
        delete this->buffer;
    }

    delete this->selectionTimer;
    delete this->lspRefreshTimer;
    if (this->currentCompleter) {
        delete this->currentCompleter;
    }
}

QFont Editor::getFont() {
    QFont font;
    #ifdef Q_OS_MAC
    font.setFamily("Inconsolata");
    font.setPointSize(15);
    #else
    font.setFamilies(QStringList() << "Source Code Pro" << "Inconsolata");
    font.setPointSize(10.5);
    #endif
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    return font;
}

void Editor::onWindowResized(QResizeEvent* event) {
    if (event != nullptr) {
        QPlainTextEdit::resizeEvent(event);
    }

    QRect cr = this->contentsRect();
    this->lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::onCursorPositionChanged() {
    this->selectionTimer->start(300);
    // update the status bar with the current line number
    this->getStatusBar()->setLineNumber(this->textCursor().blockNumber() + 1);
}

void Editor::onSelectionChanged() {
    this->selectionTimer->start(150);
}

void Editor::onTriggerLspRefresh() {
    if (this->buffer == nullptr) {
        return;
    }
    this->buffer->refreshData(this->window);
    this->window->getLSPManager()->manageBuffer(this->buffer);
    this->lspRefreshTimer->stop();
}

void Editor::onTriggerSelectionHighlight() {
    // do not highlight in big files
    if (this->document()->blockCount() > 3000) {
        return;
    }
    QTextCursor cursor = this->textCursor();
    QString text = cursor.selectedText();
    if (text.size() == 0) {
        text = "\\b" + this->getWordUnderCursor() + "\\b";
        // do not automatically highlight small words
        if (text.size() > 0 && text.size() <= 3) {
            return;
        }
    }
    this->highlightText(text);
    this->selectionTimer->stop();
}

void Editor::highlightText(QString text) {
    if (this->syntax && this->syntax->setSelection(text)) {
        this->syntax->rehighlight();
    }
}

void Editor::setSearchText(QString text) {
    if (this->syntax && this->syntax->setSearchText(text)) {
        this->syntax->rehighlight();
    }
}

void Editor::onChange(bool changed) {
    if (this->buffer != nullptr && changed) {
        this->buffer->modified = changed;
    }
    this->getStatusBar()->setModified(changed);
}

void Editor::onContentsChange(int position, int charsRemoved, int charsAdded) {
    this->lspRefreshTimer->start(500);
}

void Editor::save() {
    if (!this->buffer) { return; }
    this->buffer->save(this->window);
    this->document()->setModified(false);
    this->getStatusBar()->setModified(false);
}

void Editor::setBuffer(Buffer* buffer) {
    Q_ASSERT(buffer != nullptr);
    disconnect(this, &QPlainTextEdit::modificationChanged, this, &Editor::onChange);
    disconnect(this->document(), &QTextDocument::contentsChange, this, &Editor::onContentsChange);

    // if this editor is already responsible of a buffer,
    if (this->buffer != nullptr) {
        this->buffer->onLeave();
        this->buffer->onClose();
        // XXX(remy): may not be enough
        delete this->buffer;
        this->buffer = nullptr;
    }

    buffer->onEnter();
    this->document()->setModified(buffer->modified);
    this->window->getLSPManager()->manageBuffer(buffer);

    connect(this, &QPlainTextEdit::modificationChanged, this, &Editor::onChange);
    connect(this->document(), &QTextDocument::contentsChange, this, &Editor::onContentsChange);

    this->buffer = buffer;
    this->syntax = new SyntaxHighlighter(this, this->document());
}

QIcon Editor::getIcon() {
    if (this->bufferExtension() == "tasks") {
        return QIcon(":res/icon-check.png");
    } else if (this->bufferExtension().size() > 0) {
        QPixmap pixmap(QPixmap::fromImage(QImage(":res/icon-empty.png")));
        QPainter painter(&pixmap);
        painter.setPen(QColor(0, 0, 0));
        #ifdef Q_OS_MAC
        painter.setFont(QFont(font().family(), 240));
        #else
        painter.setFont(QFont(font().family(), 190));
        #endif
        painter.drawText(QRect(60, 75, 390, 245), Qt::AlignCenter, this->bufferExtension());
        return QIcon(pixmap);
    }
    return QIcon(":res/icon.png");
}

bool Editor::alreadyOpened(const QString& filename) {
    if (filename.size() == 0) {
        return false;
    }

    QSettings settings("mehteor", "meh");
    QFileInfo f(filename);
    int pid = settings.value("buffer/"+f.canonicalFilePath()+"/opened", 0).toInt();
    return pid != 0 && pid != QCoreApplication::applicationPid();
}

bool Editor::storeOpenedState(const QString& filename) {
    if (filename.size() == 0) {
        return false;
    }

    if (this->alreadyOpened(filename)) {
        return false; // return that we didn't update this
    }

    QSettings settings("mehteor", "meh");
    QFileInfo f(filename);
    settings.setValue("buffer/"+f.canonicalFilePath()+"/opened", QCoreApplication::applicationPid());
    return true;
}

void Editor::removeOpenedState(const QString& filename) {
    QSettings settings("mehteor", "meh");
    QFileInfo f(filename);
    settings.remove("buffer/"+f.canonicalFilePath()+"/opened");
}

void Editor::cleanOnlyWhiteSpacesLine() {
    // remove all indentation if nothing has been written on the line
    QTextCursor cursor = this->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::EndOfBlock);
    for (int v = this->currentLineIsOnlyWhitespaces(); v > 0; v--) {
        cursor.deletePreviousChar();
    }
    cursor.endEditBlock();
}

void Editor::setMode(int mode, QString command) {
    this->setOverwriteMode(false);
    switch (mode) {
    case MODE_NORMAL:
    default:
        this->getStatusBar()->setMode("NORMAL");
        // while returning in NORMAL mode, if the current line is only whitespaces
        // -> remove them.
        if (this->currentLineIsOnlyWhitespaces()) {
            this->cleanOnlyWhiteSpacesLine();
        }
        this->setBlockCursor();
        break;
    case MODE_VISUAL:
        this->getStatusBar()->setMode("VISUAL");
        this->setMidCursor();
        break;
    case MODE_VISUAL_LINE:
        this->getStatusBar()->setMode("V-LINE");
        this->visualLineBlockStart = this->textCursor().block(); // TODO(remy): is it starting with 0 or 1?
        this->moveCursor(QTextCursor::StartOfBlock);
        this->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        break;
    case MODE_INSERT:
        this->getStatusBar()->setMode("INSERT");
        this->setLineCursor();
        break;
    case MODE_COMMAND:
        this->getStatusBar()->setMode("COMMAND");
        this->setBlockCursor();
        this->window->openCommand();
        if (command.size() > 0) {
            this->window->setCommand(command);
        }
        break;
    case MODE_REPLACE:
    case MODE_REPLACE_ONE:
        this->getStatusBar()->setMode("REPLACE");
        this->setMidCursor();
        this->setOverwriteMode(true);
        break;
    case MODE_LEADER:
        this->getStatusBar()->setMode("LEADER");
        this->setMidCursor();
        this->leaderModeSelectSubMode();
        return; // we don't want to set the mode below,
                // leaderModeSelectSubMode takes care of it because it could decide
                // to go back to NORMAL if the current file doesn't fit any plugin.
    }
    this->mode = mode;
}

void Editor::setSubMode(int subMode) {
    if (subMode != NO_SUBMODE) {
        this->setMidCursor();
    }
    this->subMode = subMode;
}

//QStringList Editor::modifiedBuffers() {
//    QStringList rv;
//
//    if (this->currentBuffer != nullptr && this->currentBuffer->modified) {
//        rv << this->currentBuffer->getId();
//    }
//
//    QList<Buffer*> buff = this->buffers.values();
//    for (int i = 0; i < buff.size(); i++) {
//        if (buff.at(i)->modified) {
//            rv << buff.at(i)->getId();
//        }
//    }
//
//    return rv;
//}

void Editor::goToLine(int lineNumber) {
    // note that the findBlockByNumber starts with 0
    QTextBlock block = this->document()->findBlockByNumber(lineNumber - 1);
    QTextCursor cursor = this->textCursor();
    cursor.setPosition(block.position());
    this->setTextCursor(cursor);
    this->centerCursor();
}

void Editor::goToColumn(int column) {
    QTextCursor cursor = this->textCursor();
    if (cursor.block().text().size() <= column) {
        cursor.movePosition(QTextCursor::EndOfBlock);
        this->setTextCursor(cursor);
        return;
    }
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
    this->setTextCursor(cursor);
    this->centerCursor();
}

void Editor::centerCursor() {
    QRect cursorRect = this->cursorRect();
    // test if it's at the top
    if (cursorRect.y() < 100) {
        QScrollBar* vscroll = this->verticalScrollBar();
        vscroll->setValue(vscroll->value() - vscroll->pageStep()/2);
        return;
    }

    if (cursorRect.y() > this->rect().height() - 100) {
        QScrollBar* vscroll = this->verticalScrollBar();
        vscroll->setValue(vscroll->value() + vscroll->pageStep()/2);
        return;
    }
}

void Editor::goToOccurrence(const QString& string, bool backward) {
    QString s = string;
    QSettings settings("mehteor", "meh");
    if (string == "") {
        s = settings.value("editor/last_value_go_to_occurrence", "").toString();
    } else {
        settings.setValue("editor/last_value_go_to_occurrence", string);
    }

    // nothing has been found in this direction
    // let's try again starting from the start of the file
    // if still nothing is found, restore the cursor position
    // do this in reverse for backward
    if (backward) {
        if (!this->find(s, QTextDocument::FindBackward)) {
            QTextCursor cursor = this->textCursor();
            QTextCursor save = this->textCursor();
            cursor.movePosition(QTextCursor::End);
            this->setTextCursor(cursor);
            if (!this->find(s, QTextDocument::FindBackward)) {
                this->setTextCursor(save);
            }
        }
    } else {
        if (!this->find(s)) {
            QTextCursor cursor = this->textCursor();
            QTextCursor save = this->textCursor();
            cursor.setPosition(0);
            this->setTextCursor(cursor);
            if (!this->find(s)) {
                this->setTextCursor(save);
            }
        }
    }
}

void Editor::up() {
    this->moveCursor(QTextCursor::Up);
}

void Editor::down() {
    this->moveCursor(QTextCursor::Down);
}

void Editor::right() {
    {
        QTextCursor cursor = this->textCursor();
        QChar c = this->document()->characterAt(cursor.position()+1);
        if (c != QChar(u'\u2029')) {
            this->moveCursor(QTextCursor::Right);
        }
    }
}

void Editor::left() {
    {
        QTextCursor cursor = this->textCursor();
        QChar c = this->document()->characterAt(cursor.position()-1);
        if (c != u'\u2029') {
            this->moveCursor(QTextCursor::Left);
        }
    }
}

void Editor::moveToFirstWord(QTextCursor* cursor) {
    if (cursor == nullptr) {
        return;
    }
    QChar c = this->document()->characterAt(cursor->position());
    int watchdog = 1000;
    while (c.isSpace() && !cursor->atBlockEnd() && watchdog > 0) {
        cursor->movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
        c = this->document()->characterAt(cursor->position());
        watchdog--;
    }
}

void Editor::deleteCurrentLine() {
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void Editor::insertNewLine(bool above, bool noCutText) {
    QTextCursor cursor = this->textCursor();
    int position = cursor.position();

    if (noCutText) {
        this->moveCursor(QTextCursor::EndOfBlock);
    }

    cursor.beginEditBlock();

    // take the indent now, of the current line.
    QString indent = "";

    // if we are going up, we want the indent of n-1
    if (above) {
        int position = this->textCursor().position();
        this->moveCursor(QTextCursor::Up);
        this->moveCursor(QTextCursor::StartOfBlock);

        // special case of top of the file
        if (this->textCursor().position() == 0) {
            this->insertPlainText("\n");
            this->moveCursor(QTextCursor::Up);
            this->moveCursor(QTextCursor::StartOfBlock);
            cursor.endEditBlock();
            this->setMode(MODE_INSERT);
            return;
        }
    }

    // do we want to add extra indentation because of the previous line end of char?
    indent = this->currentLineIndent();
    QChar lastChar = this->currentLineLastChar();
    if (lastChar == ':' || lastChar == '{') {
        indent += "    ";
    }

    if (this->currentLineIsOnlyWhitespaces()) {
        this->cleanOnlyWhiteSpacesLine();
    }

    if (above) {
        cursor.setPosition(position);
        this->setTextCursor(cursor);
        this->moveCursor(QTextCursor::StartOfBlock);
        this->insertPlainText("\n");
        this->moveCursor(QTextCursor::Up);
        this->insertPlainText(indent);
    } else {
        this->insertPlainText("\n" + indent);
    }
    this->setMode(MODE_INSERT);

    cursor.endEditBlock();
}

void Editor::paintEvent(QPaintEvent* event) {
    Q_ASSERT(event != NULL);

    QPlainTextEdit::paintEvent(event);
    QPainter painter(this->viewport());
    painter.setPen(QPen(QColor(255, 255, 255, 8)));
    painter.drawLine(this->eightyCharsX, 0, this->eightyCharsX, this->viewport()->rect().height());
    painter.setPen(QPen(QColor(255, 255, 255, 3)));
    painter.drawLine(this->hundredTwentyCharsX, 0, this->hundredTwentyCharsX, this->viewport()->rect().height());
}

void Editor::mousePressEvent(QMouseEvent* event) {
    Q_ASSERT(event != NULL);

    if (event->button() == Qt::LeftButton) {
        if (this->mode == MODE_NORMAL) {
            this->setMode(MODE_INSERT);
        }
    }
    if (event->button() == Qt::MiddleButton) {
        // XXX(remy): reimplement me
        qDebug() << "Editor::mousePressEvent" << "MiddleButton";
//        if (this->currentBuffer == nullptr) {
//            return;
//        }
//
//        // position the cursor, this way, `currentLineNumber` and `currentColumn`
//        // will return the correct value.
//        QPlainTextEdit::mousePressEvent(event);
//
//        LSP* lsp = this->lspManager->getLSP(this->currentBuffer);
//        if (lsp == nullptr) {
//            this->window->getStatusBar()->setMessage("No LSP server running."); return;
//            return;
//        }
//        int reqId = QRandomGenerator::global()->generate();
//        lsp->definition(reqId, this->currentBuffer->getFilename(),
//                        this->currentLineNumber(),
//                        this->currentColumn());
//        this->lspManager->setExecutedAction(reqId, LSP_ACTION_DEFINITION, this->currentBuffer);
//        return;
    }
    if (event->button() == Qt::ForwardButton) {
        if (this->mode == MODE_NORMAL) {
            this->setMode(MODE_INSERT);
        } else if (this->mode == MODE_INSERT) {
            this->setMode(MODE_NORMAL);
        }
        return;
    }
    if (event->button() == Qt::BackButton) {
        // XXX(remy): reimplement me
         qDebug() << "Editor::mousePressEvent" << "BackButton";
//        if (this->buffers.size() > 0) {
//            // NOTE(remy): don't remove it here, just take a ref,
//            // the selectOrCreateBuffer takes care of the list order etc.
//            const QString& id = this->buffersPos.last();
//            this->selectOrCreateBuffer(id);
//        }
//        return;
    }

    QPlainTextEdit::mousePressEvent(event);
}

void Editor::keyPressEvent(QKeyEvent* event) {
    Q_ASSERT(event != NULL);

    // NOTE(remy): there is a warning about the use of QKeyEvent::modifier() in
    // the QKeyEvent class, if there is something wrong going on with the modifiers,
    // that's probably the first thing to look into.
    bool shift = event->modifiers() & Qt::ShiftModifier;
    #ifdef Q_OS_MAC
        bool ctrl = event->modifiers() & Qt::MetaModifier;
    #else
        bool ctrl = event->modifiers() & Qt::ControlModifier;
    #endif

    // First we want to test for general purpose commands and shortcuts (for instance
    // the page up/down on control-u/d).
    // ----------------------

    if (ctrl) {
        switch (event->key()) {
            case Qt::Key_U:
                {
                    QTextCursor cursor = this->textCursor();
                    cursor.beginEditBlock();
                    cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 20);
                    this->moveToFirstWord(&cursor);
                    cursor.endEditBlock();
                    this->selectionTimer->stop(); // we don't want to refresh the highlight
                    this->setTextCursor(cursor);
                }
                return;
            case Qt::Key_D:
                {
                    QTextCursor cursor = this->textCursor();
                    cursor.beginEditBlock();
                    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, 20);
                    this->moveToFirstWord(&cursor);
                    cursor.endEditBlock();
                    this->selectionTimer->stop(); // we don't want to refresh the highlight
                    this->setTextCursor(cursor);
                }
                return;
            case Qt::Key_Return:
                this->onTriggerLspRefresh();
                this->lspAutocomplete();
                return;
            case Qt::Key_R:
                this->window->openReplace();
                return;
            case Qt::Key_N:
                this->autocomplete();
                return;
            case Qt::Key_H:
                this->left();
                return;
            case Qt::Key_J:
                this->down();
                return;
            case Qt::Key_K:
                this->up();
                return;
            case Qt::Key_L:
                this->right();
                return;

            // iterate through tabs
            case Qt::Key_Up:
            case Qt::Key_Down:
                {
                    int key = event->key();
                    Editor* current = this->window->getEditor();
                    Editor* next = nullptr;

                    if (current == nullptr) { return; }

                    int tabIdx = this->window->getEditorTabIndex(current);
                    int tabsCount = this->window->getTabsCount();
                    if (key == Qt::Key_Up) {
                        if (tabIdx == 0) {
                            tabIdx = tabsCount - 1;
                        } else {
                            tabIdx--;
                        }
                    } else {
                        if (tabIdx == tabsCount-1) {
                            tabIdx = 0;
                        } else {
                            tabIdx++;
                        }
                    }
                    next = this->window->getEditor(tabIdx);
                    if (next != nullptr) {
                        this->window->setCurrentEditor(next->getId());
                    }
                }
                return;

            // delete previous word
            case Qt::Key_W:
                {
                    QTextCursor cursor = this->textCursor();
                    cursor.beginEditBlock();
                    cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
                    cursor.removeSelectedText();
                    cursor.endEditBlock();
                }
                return;

            // switch to the other buffer
            case Qt::Key_O:
                {
                    if (!this->window->getPreviousEditorId().isEmpty()) {
                        this->window->setCurrentEditor(this->window->getPreviousEditorId());
                    }
                }
                return;

            // open a file with the FilesLookup
            case Qt::Key_P:
                {
                    if (shift) {
                        // buffers list
                        this->window->openListBuffers();
                    } else {
                        // files list
                        this->window->openListFiles();
                    }
                }
                return;

            case Qt::Key_M:
                {
                    QList<QTextBlock> blocks = this->selectedBlocks();
                    if (blocks.size() == 0) {
                        blocks.append(this->textCursor().block());
                    }
                    if (shift) {
                        this->toggleComments(blocks, "#");
                    } else {
                        this->toggleComments(blocks, "//");
                    }
                }
                return;
        }
    }

    // Replace mode
    // ------------

    if (this->mode == MODE_REPLACE || this->mode == MODE_REPLACE_ONE) {
        if (event->key() == Qt::Key_Escape) {
            this->setMode(MODE_NORMAL);
            return;
        }

        QPlainTextEdit::keyPressEvent(event);
        if (this->mode == MODE_REPLACE_ONE && event->text()[0] != '\x0') {
            this->setMode(MODE_NORMAL);
            this->left();
        }
        return;
    }

    // close extra stuff
    if (event->key() == Qt::Key_Escape) {
        this->window->closeList();
        this->window->closeCompleter();
        this->window->closeReplace();
        this->window->getStatusBar()->hideMessage();
        this->setSearchText("");

        if (!this->hasFocus()) {
            this->window->closeGrep();
        } else {
            this->window->focusGrep();
        }
    }


    // Visual mode
    // -----------

    if (this->mode == MODE_VISUAL) {
        this->keyPressEventVisual(event, ctrl, shift);
        return;
    }

    if (this->mode == MODE_VISUAL_LINE) {
        this->keyPressEventVisualLine(event, ctrl, shift);
        return;
    }

    // Normal mode
    // -----------

    if (this->mode == MODE_NORMAL) {
        this->keyPressEventNormal(event, ctrl, shift);
        return;
    }

    // Leader mode
    // -----------

    if (this->mode == MODE_LEADER) {
        this->keyPressEventLeaderMode(event, ctrl, shift);
        return;
    }

    // Insert mode
    // -----------

    if (event->key() == Qt::Key_Escape) {
        QTextCursor cursor = this->textCursor();
        QChar c = this->document()->characterAt(cursor.position());
        if (c == u'\u2029') {
            this->moveCursor(QTextCursor::Left);
        }
        this->setMode(MODE_NORMAL);
        return;
    }

    if (event->key() == Qt::Key_Return) {
        this->insertNewLine(shift == true);
        return;
    }

    if (event->key() == Qt::Key_Backtab) {
        this->textCursor().insertText("\t");
        return;
    }

    if (event->key() == Qt::Key_Tab) {
        this->textCursor().insertText("    ");
        return;
    }

    if (event->text() == "}") {
        if (this->currentLineIsOnlyWhitespaces() >= 0) {
            this->removeIndentation(this->textCursor());
        }
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    // Otherwise, rely on the original behavior of QPlainTextEdit
    QPlainTextEdit::keyPressEvent(event);
}

void Editor::toggleComments(QList<QTextBlock> blocks, const QString& commentChars) {
    QTextCursor cursor = this->textCursor();

    cursor.beginEditBlock();
    for (int i = 0; i < blocks.size(); i++) {
        QTextBlock block = blocks.at(i);
        QString text = block.text();

        int commentPos = text.indexOf(commentChars);

        // we still want to be sure that it is as the start of the line
        if (commentPos != -1 && text.startsWith(commentChars)) {
            // uncomment
            cursor.setPosition(block.position() + commentPos);
            for (int j = 0; j < commentChars.size(); j++) {
                cursor.deleteChar();
            }
        } else {
            // comment
            cursor.setPosition(block.position());
            cursor.insertText(commentChars);
        }
    }
    cursor.endEditBlock();
}

void Editor::insertIndentation(QTextCursor cursor) {
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText("    ");
}

void Editor::removeIndentation(QTextCursor cursor) {
    // remove one layer of indentation
    cursor.movePosition(QTextCursor::StartOfLine);
    int i = 0;
    for (i = 0; i < 4; i++) {
        QChar c = this->document()->characterAt(cursor.position());
        if (c == ' ') {
            cursor.deleteChar();
        } else if (c == '\t') {
            cursor.deleteChar();
            i++;
            break;
        } else {
            break;
        }
    }
}

QString Editor::currentLineIndent() {
    QString text = this->textCursor().block().text();
    QString rv;
    for(int i = 0; i < text.size(); i++) {
        if (text.at(i) == ' '  || text.at(i) == '\t') {
            rv.append(text.at(i));
        } else {
            // stop as soon as something else has been found
            break;
        }
    }
    return rv;
}

int Editor::currentLineNumber() {
    return this->textCursor().blockNumber() + 1;
}

int Editor::currentColumn() {
    return this->textCursor().positionInBlock();
}

int Editor::currentLineIsOnlyWhitespaces() {
    QString text = this->textCursor().block().text();
    int count = 0;
    for(int i = 0; i < text.size(); i++) {
        QChar c = text.at(i);
        if (c != ' '  && c != '\t' && c != '\n') {
            return -1;
        }
        if (c == '\n') { break; }
        count++;
    }
    return count;
}

QChar Editor::currentLineLastChar() {
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock);
    return QChar(this->document()->characterAt(cursor.position()-1));
}

QList<QTextBlock> Editor::selectedBlocks() {
    QList<QTextBlock> rv;

    QTextCursor cursor = this->textCursor();
    if (!cursor.hasSelection()) {
        return rv;
    }

    QTextBlock start = this->document()->findBlock(cursor.selectionStart());
    QTextBlock end = this->document()->findBlock(cursor.selectionEnd());

    while (true) {
        rv.append(start);
        if (start == end) {
            break;
        }
        start = start.next();
    }

    return rv;
}

QString Editor::getWordUnderCursor() {
    QString rv;

    QTextCursor cursor = this->textCursor();
    QString text = cursor.block().text();

    int pos = cursor.positionInBlock();
    bool leftOnly = false;
    if (text[pos].isNumber() || text[pos].isLetter() || text[pos] == '_') {
        rv.append(text[pos]);
    } else {
        // we may be at the end of a word, on a point or anything, in this case
        // use only the left of the cursor.
        leftOnly = true;
    }

    // go left first, if we find something will go with that
    for (int i = pos-1; i >= 0; i--) {
        if (text[i].isNumber() || text[i].isLetter() || text[i] == '_') {
            rv.prepend(text[i]);
            continue;
        }
        break;
    }

    if (!leftOnly) {
        for (int i = pos+1; i <= text.size(); i++) {
            if (text[i].isNumber() || text[i].isLetter() || text[i] == '_') {
                rv.append(text[i]);
                continue;
            }
            break;
        }
    }

    return rv;
}

void Editor::lspAutocomplete() {
    LSPManager* manager = this->window->getLSPManager();
    if (manager == nullptr) {
        return;
    }

    LSP* lsp = manager->getLSP(this->getId());
    int reqId = QRandomGenerator::global()->generate();
    if (reqId < 0) { reqId *= -1; }

    if (lsp == nullptr) {
        this->window->getStatusBar()->setMessage("No LSP server running for this buffer.");
        return;
    }

    lsp->completion(reqId, this->buffer->getFilename(), this->currentLineNumber(), this->currentColumn());
    manager->setExecutedAction(reqId, LSP_ACTION_COMPLETION, this->buffer);
}

void Editor::autocomplete() {
    const QString& base = this->getWordUnderCursor();

    QRegularExpression rx = QRegularExpression("("+base + "\\w+)");

    QStringList list;

    QRegularExpressionMatchIterator it = rx.globalMatch(this->document()->toPlainText());
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        list << match.captured();
    }

    QList<Editor*> editors = this->window->getEditors();
    for (int i = 0; i < editors.size(); i++) {
        QRegularExpressionMatchIterator it = rx.globalMatch(editors[i]->getBuffer()->getData());
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            list << match.captured();
        }
    }

    list.removeDuplicates();

    if (list.size() == 1) {
        this->applyAutocomplete(base, list[0]);
        return;
    }

    if (list.size() == 0) {
        return;
    }

    this->selectionTimer->stop();

    QList<CompleterEntry> entries;
    for (int i = 0; i < list.size(); i++) {
        entries.append(CompleterEntry(list[i], ""));
    }
    this->window->openCompleter(base, entries);
}

void Editor::applyAutocomplete(const QString& base, const QString& word) {
    QTextCursor cursor = this->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, base.size());
    cursor.deleteChar();
    cursor.insertText(word);
    cursor.endEditBlock();
}

int Editor::findPreviousOneInCurrentLine(QChar c) {
    QTextCursor cursor = this->textCursor();
    QString text = cursor.block().text();

    if (cursor.positionInBlock() == 0) { return 0; }

    for (int i = cursor.positionInBlock()-1; i >= 0; i--) {
        if (text[i] == c) {
            return cursor.positionInBlock() - i;
        }
    }

    return 0;
}

QString Editor::bufferExtension() {
    if (this->buffer == nullptr) {
        return "";
    }

    QStringList parts = this->buffer->getFilename().split(".");
    if (parts.size() == 1) {
        return "";
    }

    return parts[parts.size() - 1];
}

int Editor::findNextOneInCurrentLine(QChar c) {
    QTextCursor cursor = this->textCursor();
    QString text = cursor.block().text();
    for (int i = cursor.positionInBlock()+1; i < text.size(); i++) {
        if (text[i] == c) {
            return i - cursor.positionInBlock();
        }
    }
    return 0;
}

StatusBar* Editor::getStatusBar() {
    return this->window->getStatusBar();
}

const QString Editor::getOneLine(const QString filename, int line) {
    QFile file(filename);
    QString rv;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        for (int i = 0; i < line-1 && !file.atEnd(); i++) { file.readLine(); }
        rv = file.readLine().trimmed();
    }
    return rv;
}

// line area
// ---------

void Editor::onUpdateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::onUpdateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        this->lineNumberArea->scroll(0, dy);
    } else {
        this->lineNumberArea->update(0, rect.y(), this->lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        this->onUpdateLineNumberAreaWidth(0);
    }
}

void Editor::update() {
    // FIXME(remy): trick to have the line area number redrawn
    this->onWindowResized(nullptr);
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    Q_ASSERT(this->window != nullptr);

    if (this->buffer == nullptr) {
        return;
    }

    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor::fromRgb(30, 30, 30));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    QMap<int, QList<LSPDiagnostic>> diags = this->window->getLSPManager()->getDiagnostics(this->buffer->getFilename());

    int currentLine = this->currentLineNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);

            // background
            if (diags.contains(blockNumber + 1)) {
              // use differents red if it's on the current line or not
              if (currentLine == blockNumber+1) {
                painter.fillRect(0, top, this->window->width(), fontMetrics().height(), QColor(100,30,30));
              } else {
                painter.fillRect(0, top, this->window->width(), fontMetrics().height(), QColor(70,30,30));
              }
            } else if (currentLine == blockNumber+1) {
                painter.fillRect(0, top, lineNumberArea->width(), fontMetrics().height(), this->highlightedLine);
            }

            // foreground
            if (diags.contains(blockNumber + 1)) {
                painter.setPen(QColor::fromRgb(150, 150, 150));
            } else if (currentLine == blockNumber+1) {
                painter.setPen(QColor::fromRgb(200, 200, 200));
            } else {
                painter.setPen(QColor::fromRgb(50, 50, 50));
            }

            painter.setFont(Editor::getFont());
            painter.drawText(0, top, lineNumberArea->width()-2, fontMetrics().height(), Qt::AlignCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int Editor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    if (digits < 5) { digits = 5; }

    int space = 5 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

int Editor::lineNumberAtY(int y) {
    QTextBlock block = this->firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(this->blockBoundingGeometry(block).translated(this->contentOffset()).top());
    int bottom = top + qRound(this->blockBoundingRect(block).height());

    while (block.isValid()) {
        block = block.next();
        if (y >= top && y <= bottom) {
            return block.blockNumber();
        }
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    return -1;
}
