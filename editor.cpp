#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
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
#include "mode.h"
#include "references_widget.h"
#include "syntax.h"
#include "tasks.h"
#include "window.h"

Editor::Editor(Window* window) :
    tasksPlugin(nullptr),
    QPlainTextEdit(window),
    currentCompleter(nullptr),
    window(window),
    currentBuffer(nullptr),
    mode(MODE_NORMAL),
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

    // syntax highlighting
    // ----------------------

    this->syntax = new Syntax(this->document());

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
    if (this->currentBuffer != nullptr) {
        this->currentBuffer->onLeave(this); // store settings
        this->currentBuffer->onClose(this);
        delete this->currentBuffer;
    }
    QList<Buffer*> buf = this->buffers.values();
    for (int i = 0; i < buf.size(); i++) {
        if (buf.at(i) != nullptr) {
            buf.at(i)->onLeave(this);
            buf.at(i)->onClose(this);
            delete buf.at(i);
        }
    }

    delete this->selectionTimer;
    delete this->lspRefreshTimer;
    if (this->currentCompleter) {
        delete this->currentCompleter;
    }
}

QFont Editor::getFont() {
    QFont font;
    font.setFamily("Inconsolata");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    #ifdef Q_OS_MAC
    font.setPointSize(13);
    #else
    font.setPointSize(11);
    #endif
    return font;
}

void Editor::onWindowResized(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);

    QRect cr = this->contentsRect();
    this->lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::onCursorPositionChanged() {
    this->selectionTimer->start(1000);

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(this->highlightedLine);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    this->setExtraSelections(extraSelections);

    // NOTE(remy): block number should not only be equals to line number...
    this->getStatusBar()->setLineNumber(this->textCursor().blockNumber() + 1);
}

void Editor::onSelectionChanged() {
    this->selectionTimer->start(500);
}

void Editor::onTriggerLspRefresh() {
    if (this->currentBuffer == nullptr) {
        return;
    }
    this->currentBuffer->refreshData(this);
    this->lspManager.manageBuffer(this->window, this->currentBuffer);
    this->lspRefreshTimer->stop();
}

void Editor::onTriggerSelectionHighlight() {
    if (this->document()->blockCount() > 3000) {
        return;
    }
    QTextCursor cursor = this->textCursor();
    QString text = cursor.selectedText();
    if (text.size() == 0) {
        text = this->getWordUnderCursor();
        // do not automatically highlight small words
        if (text.size() > 0 && text.size() <= 3) {
            return;
        }
    }
    this->highlightText(text);
    this->selectionTimer->stop();
}

void Editor::highlightText(QString text) {
    if (this->syntax->setSelection(text)) {
        this->syntax->rehighlight();
    }
}

void Editor::onChange(bool changed) {
    if (this->currentBuffer != nullptr && changed) {
        this->currentBuffer->modified = changed;
    }
    this->getStatusBar()->setModified(changed);
}

void Editor::onContentsChange(int position, int charsRemoved, int charsAdded) {
    this->lspRefreshTimer->start(500);
}


void Editor::save() {
    if (!this->currentBuffer) { return; }

    this->currentBuffer->save(this);
    this->document()->setModified(false);
    this->getStatusBar()->setModified(false);
}

void Editor::saveAll() {
    // save the current buffer.
    this->save();

    // save other buffers
    QList<Buffer*> buff = this->buffers.values();
    for (int i = 0; i < buff.size(); i++) {
        if (buff.at(i)->modified) {
            buff.at(i)->save(this);
        }
    }

    this->document()->setModified(false);
    this->getStatusBar()->setModified(false);
}

void Editor::setCurrentBuffer(Buffer* buffer) {
    Q_ASSERT(buffer != NULL);
    disconnect(this, &QPlainTextEdit::modificationChanged, this, &Editor::onChange);
    disconnect(this->document(), &QTextDocument::contentsChange, this, &Editor::onContentsChange);
    if (this->currentBuffer != nullptr) {
        this->currentBuffer->onLeave(this);
        // we're leaving this one, append it to the end of the buffers list.
        this->buffersPos.append(this->currentBuffer->getFilename());
        this->buffers[this->currentBuffer->getFilename()] = this->currentBuffer;
    }

    this->currentBuffer = buffer;
    this->currentBuffer->onEnter(this);
    this->document()->setModified(this->currentBuffer->modified);

    if (this->currentBufferExtension() == "tasks") {
        this->window->setWindowIcon(QIcon(":res/icon-check.png"));
    } else if (this->currentBufferExtension().size() > 0) {
        QPixmap pixmap(QPixmap::fromImage(QImage(":res/icon-empty.png")));
        QPainter painter(&pixmap);
        painter.setPen(QColor(0, 0, 0));
        #ifdef Q_OS_MAC
        painter.setFont(QFont(font().family(), 160));
        #else
        painter.setFont(QFont(font().family(), 140));
        #endif
        painter.drawText(QRect(60, 75, 390, 245), Qt::AlignCenter, this->currentBufferExtension());
        this->window->setWindowIcon(QIcon(pixmap));
    } else {
        this->window->setWindowIcon(QIcon(":res/icon.png"));
    }

    connect(this, &QPlainTextEdit::modificationChanged, this, &Editor::onChange);
    connect(this->document(), &QTextDocument::contentsChange, this, &Editor::onContentsChange);
}

void Editor::selectOrCreateBuffer(const QString& filename) {
    QFileInfo info(filename);
    QString f = info.absoluteFilePath();

    // do not do anything if the current file is the buffer we try to open
    if (this->currentBuffer != nullptr && this->currentBuffer->getFilename() == f) {
        return;
    }

    Buffer* buffer = this->buffers.take(f);
    if (buffer == nullptr) {
        // check that this file has not been opened by another instance
        // of the editor.
        if (this->alreadyOpened(filename)) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Already opened");
            msgBox.setText("This file is already opened by another instance of meh, do you still want to open it?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            if (msgBox.exec() == QMessageBox::Cancel) {
                return;
            }
        }

        // this file has never been opened, open it
        buffer = new Buffer(f);
        // store somewhere that it is now open by someone
        this->storeOpenedState(filename);
    } else {
        int pos = this->buffersPos.indexOf(f);
        if (pos >= 0) { // should not happen
            this->buffersPos.remove(pos);
        } else {
            qDebug() << "pos == -1 in selectBuffer, should never happen.";
        }
    }

    this->window->setWindowTitle("meh - " + filename);
    this->getStatusBar()->setFilename(filename);
    this->setCurrentBuffer(buffer);
    lspManager.manageBuffer(this->window, buffer);
}

void Editor::closeCurrentBuffer() {
    if (this->currentBuffer == nullptr) {
        return;
    }

    this->currentBuffer->onLeave(this); // store settings
    this->currentBuffer->onClose(this);
    delete this->currentBuffer;
    this->currentBuffer = nullptr;

    if (this->buffers.size() == 0) {
        this->setPlainText(""); // clear
        this->window->setWindowTitle("meh - no file");
        return;
    }

    // NOTE(remy): don't remove it here, just take a ref,
    // the selectOrCreateBuffer takes care of the list order etc.
    const QString& filename = this->buffersPos.last();
    this->selectOrCreateBuffer(filename);
}

bool Editor::hasBuffer(const QString& filename) {
    if (this->currentBuffer == nullptr) {
        return false;
    }

    QFileInfo info(filename);
    return this->currentBuffer->getFilename() == info.absoluteFilePath() ||
            this->buffers.contains(info.absoluteFilePath());
}

bool Editor::alreadyOpened(const QString& filename) {
    if (filename.size() == 0) {
        return false;
    }

    QSettings settings("mehteor", "meh");
    QFileInfo f(filename);
    int pid = settings.value("buffer/"+f.absoluteFilePath()+"/opened", 0).toInt();
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
    settings.setValue("buffer/"+f.absoluteFilePath()+"/opened", QCoreApplication::applicationPid());
    return true;
}

void Editor::removeOpenedState(const QString& filename) {
    QSettings settings("mehteor", "meh");
    QFileInfo f(filename);
    settings.remove("buffer/"+f.absoluteFilePath()+"/opened");
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
            // remove all indentation if nothing has been written on the line
            QTextCursor cursor = this->textCursor();
            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::EndOfBlock);
            for (int v = this->currentLineIsOnlyWhitespaces(); v > 0; v--) {
                cursor.deletePreviousChar();
            }
            cursor.endEditBlock();
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

QStringList Editor::modifiedBuffers() {
    QStringList rv;

    if (this->currentBuffer != nullptr && this->currentBuffer->modified) {
        rv << this->currentBuffer->getFilename();
    }

    QList<Buffer*> buff = this->buffers.values();
    for (int i = 0; i < buff.size(); i++) {
        if (buff.at(i)->modified) {
            rv << buff.at(i)->getFilename();
        }
    }

    return rv;
}

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
        if (c != "\u2029") {
            this->moveCursor(QTextCursor::Right);
        }
    }
}

void Editor::left() {
    {
        QTextCursor cursor = this->textCursor();
        QChar c = this->document()->characterAt(cursor.position()-1);
        if (c != "\u2029") {
            this->moveCursor(QTextCursor::Left);
        }
    }
}

void Editor::deleteCurrentLine() {
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void Editor::insertNewLine(bool above) {
    // take the indent now, of the current line.
    if (above) {
        this->moveCursor(QTextCursor::Up);
    }
    this->moveCursor(QTextCursor::EndOfLine);
    QString indent = this->currentLineIndent();

    QChar lastChar = this->currentLineLastChar();
    if (lastChar == ":" || lastChar == "{") {
        indent += "    ";
    }
    this->insertPlainText("\n" + indent);
    this->setMode(MODE_INSERT);
    return;
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
    if (event->button() == Qt::ForwardButton) {
        if (this->mode == MODE_NORMAL) {
            this->setMode(MODE_INSERT);
        } else if (this->mode == MODE_INSERT) {
            this->setMode(MODE_NORMAL);
        }
        return;
    }
    if (event->button() == Qt::BackButton) {
        if (this->buffers.size() > 0) {
            // NOTE(remy): don't remove it here, just take a ref,
            // the selectOrCreateBuffer takes care of the list order etc.
            const QString& filename = this->buffersPos.last();
            this->selectOrCreateBuffer(filename);
        }
        return;
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
                    QKeyEvent pageEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier);
                    QPlainTextEdit::keyPressEvent(&pageEvent);
                }
                return;
            case Qt::Key_D:
                {
                    QKeyEvent pageEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
                    QPlainTextEdit::keyPressEvent(&pageEvent);
                }
                return;
            case Qt::Key_Space:
                this->onTriggerLspRefresh();
                this->lspAutocomplete();
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
                    if (this->buffersPos.size() == 0) {
                        return;
                    }
                    // NOTE(remy): don't remove it here, just take a ref,
                    // the selectOrCreateBuffer takes care of the list order etc.
                    const QString& filename = this->buffersPos.last();
                    this->selectOrCreateBuffer(filename);
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

    // close extra widgets
    if (event->key() == Qt::Key_Escape) {
        this->window->closeList();
        this->window->closeCompleter();
        this->window->getStatusBar()->hideMessage();

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
        if (c == "\u2029") {
            this->moveCursor(QTextCursor::Left);
        }
        this->setMode(MODE_NORMAL);
        return;
    }

    if (event->key() == Qt::Key_Return) {
        QString indent = this->currentLineIndent();
        QTextCursor cursor = this->textCursor();

        cursor.beginEditBlock();

        // remove all indentation if nothing has been written on the line
        for (int v = this->currentLineIsOnlyWhitespaces(); v > 0; v--) {
            cursor.deletePreviousChar();
        }

        if (shift) {
            this->moveCursor(QTextCursor::Up);
            this->moveCursor(QTextCursor::EndOfLine);
        } else {
            // FIXME(remy): doesn't work propery for Shift+Return.
            QChar lastChar = this->currentLineLastChar();
            if (lastChar == ":" || lastChar == "{") {
                indent += "    ";
            }

            // If next chars are white spaces we'll want to remove them before
            // going to the next line.
            QChar c = this->document()->characterAt(cursor.position());
            while (c == " " || c == "\t") {
                cursor.deleteChar();
                c = this->document()->characterAt(cursor.position());
            }
        }

        this->insertPlainText("\n" + indent);
        cursor.endEditBlock();
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
        if (commentPos != -1 && text.trimmed().startsWith(commentChars)) {
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
        if (c == " ") {
            cursor.deleteChar();
        } else if (c == "\t") {
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
    return this->textCursor().blockNumber();
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
    LSP* lsp = this->window->getEditor()->lspManager.getLSP(currentBuffer);
    int reqId = QRandomGenerator::global()->generate();
    if (reqId < 0) { reqId *= -1; }

    if (lsp == nullptr) {
        this->window->getStatusBar()->setMessage("No LSP server running for this buffer.");
        return;
    }
    lsp->completion(reqId, currentBuffer->getFilename(), this->currentLineNumber(), this->currentColumn());
    this->lspManager.setExecutedAction(reqId, LSP_ACTION_COMPLETION, currentBuffer);
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

    QList<Buffer*> buffers = this->buffers.values();
    for (int i = 0; i < buffers.size(); i++) {
        QRegularExpressionMatchIterator it = rx.globalMatch(buffers[i]->getData());
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

QString Editor::currentBufferExtension() {
    if (this->currentBuffer == nullptr) {
        return "";
    }

    QStringList parts = this->currentBuffer->getFilename().split(".");
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

void Editor::lspInterpret(QByteArray data) {
    QJsonDocument json = LSPReader::readMessage(data);
    if (json.isEmpty()) {
        return;
    }

    LSPAction action = this->lspManager.getExecutedAction(json["id"].toInt());
    if (action.requestId == 0) {
        if (json["method"].isNull()) {
            return;
        }
        // showMessage
        // -----------
        if (json["method"].toString() == "window/showMessage") {
            if (!json["params"].isNull()) {
                const QString& msg = json["params"]["message"].toString();
                if (msg.size() > 0) {
                    this->window->getStatusBar()->setMessage(msg);
                }
            }
        // publishDiagnostics
        // ------------------
        } else if (json["method"] == "textDocument/publishDiagnostics") {
            // TODO(remy): implement me at some point
            if (!json["params"].isNull() && !json["params"]["diagnostics"].isNull()) {
                if (!json["params"]["diagnostics"].isArray()) {
                    qWarning() << "lspInterpret: \"diagnostics\" is not an array";
                    return;
                }
                if (json["params"]["uri"].isNull()) {
                    qWarning() << "lspInterpret: no \"uri\" field in diagnostic";
                    return;
                }

                auto diags = json["params"]["diagnostics"].toArray();
                const QString& uri = QFileInfo(json["params"]["uri"].toString().replace("file://", "")).absoluteFilePath();

                if (diags.size() == 0) {
                    this->lspManager.clearDiagnostics(uri);
                    this->repaint();
                    return;
                }

                for (int i = 0; i < diags.size(); i++) {
                    QJsonObject diag = diags[i].toObject();
                    const QString& msg = diag["message"].toString();
                    if (!diag["range"].isNull() && !diag["range"].toObject()["start"].isNull()
                            && !diag["range"].toObject()["start"].toObject()["line"].isNull()) {
                        int line = diag["range"].toObject()["start"].toObject()["line"].toInt();
                        LSPDiagnostic diag;
                        diag.line = line + 1;
                        diag.message = msg;
                        diag.absFilename = uri;
                        this->lspManager.addDiagnostic(uri, diag);
                    }
                }
                this->repaint();
            }
        }
        return;
    }

    switch (action.action) {
        case LSP_ACTION_DECLARATION:
        case LSP_ACTION_DEFINITION:
            {
                // TODO(remy): deal with multiple results
                int line = json["result"][0]["range"]["start"]["line"].toInt();
                int column = json["result"][0]["range"]["start"]["character"].toInt();
                QString file = json["result"][0]["uri"].toString();
                if (file.isEmpty()) {
                    this->window->getStatusBar()->setMessage("Nothing found.");
                    return;
                }
                file.remove(0,7); // remove the file://
                this->saveCheckpoint();
                this->selectOrCreateBuffer(file);
                this->goToLine(line + 1);
                this->goToColumn(column);
                return;
            }
        case LSP_ACTION_COMPLETION:
            {
                if (json["result"].isNull()) {
                    this->window->getStatusBar()->setMessage("Nothing found.");
                    return;
                }

                LSP* lsp = this->lspManager.getLSP(this->currentBuffer) ;
                if (lsp == nullptr) {
                    this->window->getStatusBar()->setMessage("Nothing found.");
                    return;
                }

                auto entries = lsp->getEntries(json);
                if (entries.size() == 0) {
                    this->window->getStatusBar()->setMessage("Nothing found.");
                    return;
                }
                const QString& base = this->getWordUnderCursor();
                this->window->openCompleter(base, entries);
                return;
            }
        case LSP_ACTION_HOVER:
            {
                if (json["result"].isNull() || json["result"]["contents"].isNull()) {
                    this->window->getStatusBar()->setMessage("Nothing found.");
                    return;
                }

                auto contents = json["result"]["contents"].toObject();
                this->window->getStatusBar()->setMessage(contents["value"].toString());
                return;
            }
        case LSP_ACTION_SIGNATURE_HELP:
            {
                if (!json["result"].isNull() && !json["result"]["signatures"].isNull() && json["result"]["signatures"].toArray().size() > 0) {
                    QString message;
                    QJsonArray signatures = json["result"]["signatures"].toArray();
                    for (int i = 0; i < signatures.size(); i++) {
                        QJsonValue signature = signatures[i];
                        message.append(signature["label"].toString()).append("\n");
                        message.append(signature["documentation"].toString());
                    }
                    this->window->getStatusBar()->setMessage(message);
                    return;
                }
                this->window->getStatusBar()->setMessage("Nothing found.");
                return;
            }
        case LSP_ACTION_REFERENCES:
            {
                // TODO(remy): error management
                this->window->getRefWidget()->clear();
                this->window->getRefWidget()->hide();
                QJsonArray list = json["result"].toArray();
                for (int i = 0; i < list.size(); i++) {
                    QJsonObject entry = list[i].toObject();
                    int line = entry["range"].toObject()["start"].toObject()["line"].toInt();
                    line += 1;
                    QString file = entry["uri"].toString();
                    if (file.startsWith("file://")) {
                        file = file.remove(0, 7);
                    }
                    if (file.startsWith(this->window->getBaseDir())) {
                        file = file.remove(0, this->window->getBaseDir().size());
                    }
                    this->window->getRefWidget()->insert(file, QString::number(line), "");
                }
                this->window->getRefWidget()->fitContent();
                this->window->getRefWidget()->show();
                this->window->getRefWidget()->setFocus();
                return;
            }
    }
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

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    if (this->currentBuffer == nullptr) {
        return;
    }

    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor::fromRgb(30, 30, 30));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    QMap<int, LSPDiagnostic> diags = this->lspManager.getDiagnostics(this->currentBuffer->getFilename());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            if (diags.contains(blockNumber + 1)) {
                painter.setPen(QColor::fromRgb(250, 50, 50));
            } else {
                painter.setPen(QColor::fromRgb(50, 50, 50));
            }
            painter.drawText(0, top, lineNumberArea->width()-2, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int Editor::lineNumberAreaWidth() {
    // TODO(remy): I don't want this to be dynamic

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    if (digits < 5) { digits = 5; }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}


// checkpoints
// -----------

void Editor::saveCheckpoint() {
    if (this->getCurrentBuffer() == nullptr) {
        return;
    }

    QString filename = this->getCurrentBuffer()->getFilename();
    int position = this->textCursor().position();
    Checkpoint c(filename, position);

    if (this->checkpoints.isEmpty()) {
        this->checkpoints.append(c);
        return;
    }

    if (!this->checkpoints.isEmpty()) {
        Checkpoint last = this->checkpoints.last();
        if (last.filename != filename || last.position != position) {
            this->checkpoints.append(c);
        }
    }
}

void Editor::lastCheckpoint() {
    if (!this->checkpoints.isEmpty()) {
        Checkpoint c = this->checkpoints.takeLast();
        this->selectOrCreateBuffer(c.filename);
        QTextCursor cursor = this->textCursor();
        cursor.setPosition(c.position);
        this->setTextCursor(cursor);
    }
}
