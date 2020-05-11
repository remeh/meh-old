#include <QColor>
#include <QKeyEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPaintEvent>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>

#include "qdebug.h"

#include "editor.h"
#include "mode.h"
#include "syntax.h"
#include "window.h"

Editor::Editor(Window* window) :
    QTextEdit(window),
    currentCompleter(nullptr),
    window(window),
    currentBuffer(NULL),
    mode(MODE_NORMAL),
    highlightedLine(QColor::fromRgb(50, 50, 50)) {
    Q_ASSERT(window != NULL);

    // we don't want a rich text editor
    // ----------------------

    this->setAcceptRichText(false);

    // editor font
    // ----------------------

    QFont font;
    font.setFamily("Inconsolata");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    #ifdef Q_OS_MAC
    font.setPointSize(14);
    #else
    font.setPointSize(11);
    #endif
    this->setFont(font);

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

    QFont labelFont = font;
    labelFont.setPointSize(14);
    this->modeLabel = new QLabel("NORMAL  ", this);
    this->modeLabel->setStyleSheet("color: rgba(255, 255, 255, 30); background-color: rgba(0, 0, 0, 0);");
    this->modeLabel->setFont(labelFont);
    this->modeLabel->show();

    this->lineLabel = new QLabel("       1", this);
    this->lineLabel->setStyleSheet("color: rgba(255, 255, 255, 30); background-color: rgba(0, 0, 0, 0);");
    this->lineLabel->setFont(labelFont);
    this->lineLabel->show();

    this->modifiedLabel = new QLabel("", this);
    this->modifiedLabel->setStyleSheet("color: rgba(255, 255, 255, 30); background-color: rgba(0, 0, 0, 0);");
    this->modifiedLabel->setFont(labelFont);
    this->modifiedLabel->show();

    // tab space size
    // ----------------------

    const int tabSpace = 4;
    QFontMetrics metrics(font);
    this->setTabStopDistance(tabSpace*metrics.horizontalAdvance(" "));

    // 80 chars separator
    // ----------------------

    QString eightyChars;
    for (int i = 0; i < 80; i++) { eightyChars += "#"; }
    this->eightyCharsX = metrics.horizontalAdvance(eightyChars);

    // start in normal mode
    // ----------------------

    this->setSubMode(NO_SUBMODE);
    this->setMode(MODE_NORMAL);

    connect(this, &QTextEdit::selectionChanged, this, &Editor::onSelectionChanged);
    connect(this, &QTextEdit::cursorPositionChanged, this, &Editor::onCursorPositionChanged);
    connect(this->selectionTimer, &QTimer::timeout, this, &Editor::onTriggerSelectionHighlight);
}

Editor::~Editor() {
    if (this->currentBuffer != nullptr) {
        this->currentBuffer->onLeave(this); // store settings
        delete this->currentBuffer;
    }
    QList<Buffer*> buf = this->buffers.values();
    for (int i = 0; i < buf.size(); i++) {
        if (buf.at(i) != nullptr) {
            buf.at(i)->onLeave(this);
            delete buf.at(i);
        }
    }

    delete this->selectionTimer;
    delete this->modeLabel;
    delete this->lineLabel;
    delete this->modifiedLabel;
    if (this->currentCompleter) {
        delete this->currentCompleter;
    }
}

void Editor::onWindowResized(QResizeEvent*) {
    int x = this->window->rect().width() - 90;
    this->modeLabel->move(x, 2);
    this->lineLabel->move(x, 20);
    this->modifiedLabel->move(x, 38);
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
    this->lineLabel->setText(QString::number(this->textCursor().blockNumber() + 1));
}

void Editor::onSelectionChanged() {
    this->selectionTimer->start(500);
}

void Editor::onTriggerSelectionHighlight() {
    if (this->document()->blockCount() > 3000) {
        return;
    }
    QTextCursor cursor = this->textCursor();
    QString text = cursor.selectedText();
    if (text.size() == 0) {
        text = this->getWordUnderCursor();
    }
    if (this->syntax->setSelection(text)) {
        this->syntax->rehighlight();
    }
    this->selectionTimer->stop();
}

void Editor::onChange(bool changed) {
    if (this->currentBuffer != nullptr && changed) {
        this->currentBuffer->modified = true;
        this->modifiedLabel->setText("*");
    }
}

void Editor::save() {
    if (!this->currentBuffer) { return; }

    this->currentBuffer->save(this);
    this->modifiedLabel->setText(" ");
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

    this->modifiedLabel->setText(" ");
}

void Editor::setCurrentBuffer(Buffer* buffer) {
    Q_ASSERT(buffer != NULL);
    disconnect(this->document(), &QTextDocument::modificationChanged, this, &Editor::onChange);
    if (this->currentBuffer != nullptr) {
        this->currentBuffer->onLeave(this);
        // we're leaving this one, append it to the end of the buffers list.
        this->buffersPos.append(this->currentBuffer->getFilename());
        this->buffers[this->currentBuffer->getFilename()] = this->currentBuffer;
    }

    this->currentBuffer = buffer;
    this->currentBuffer->onEnter(this);
    this->document()->setModified(false);
    if (this->currentBuffer->modified) {
        this->modifiedLabel->setText("*");
    } else {
        this->modifiedLabel->setText("");
    }
    connect(this->document(), &QTextDocument::modificationChanged, this, &Editor::onChange);
}

void Editor::selectOrCreateBuffer(const QString& filename) {
    QFileInfo info(filename);
    QString f = info.absoluteFilePath();

    Buffer* buffer = this->buffers.take(f);
    if (buffer == nullptr) {
        // this file has never been opened, open it
        buffer = new Buffer(f);
    } else {
        int pos = this->buffersPos.indexOf(f);
        if (pos >= 0) { // should not happen
            this->buffersPos.remove(pos);
        } else {
            qDebug() << "pos == -1 in selectBuffer, should never happen.";
        }
    }

    this->window->setWindowTitle("meh - " + filename);
    this->setCurrentBuffer(buffer);
}

bool Editor::hasBuffer(const QString& filename) {
    if (this->currentBuffer == nullptr) {
        return false;
    }

    QFileInfo info(filename);
    return this->currentBuffer->getFilename() == info.absoluteFilePath() ||
            this->buffers.contains(info.absoluteFilePath());
}

void Editor::setMode(int mode, QString command) {
    this->setOverwriteMode(false);
    switch (mode) {
    case MODE_NORMAL:
    default:
        this->modeLabel->setText("NORMAL");
        this->setBlockCursor();
        break;
    case MODE_VISUAL:
        this->modeLabel->setText("VISUAL");
        this->setMidCursor();
        break;
    case MODE_VISUAL_LINE:
        this->modeLabel->setText("V-LINE");
        this->moveCursor(QTextCursor::StartOfBlock);
        this->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        break;
    case MODE_INSERT:
        this->modeLabel->setText("INSERT");
        this->setLineCursor();
        break;
    case MODE_COMMAND:
        this->modeLabel->setText("COMMAND");
        this->setBlockCursor();
        this->window->openCommand();
        if (command.size() > 0) {
            this->window->setCommand(command);
        }
        break;
    case MODE_REPLACE:
    case MODE_REPLACE_ONE:
        this->modeLabel->setText("REPLACE");
        this->setMidCursor();
        this->setOverwriteMode(true);
        break;
    case NO_SUBMODE:
        this->modeLabel->setText("");
    }
    this->mode = mode;
}

void Editor::setSubMode(int subMode) {
    if (subMode != NO_SUBMODE) {
        this->setMidCursor();
    }
    switch (subMode) {
    case SUBMODE_c:
        this->modeLabel->setText("c");
        break;
    case SUBMODE_cf:
        this->modeLabel->setText("cf");
        break;
    case SUBMODE_cF:
        this->modeLabel->setText("cF");
        break;
    case SUBMODE_ct:
        this->modeLabel->setText("ct");
        break;
    case SUBMODE_cT:
        this->modeLabel->setText("cT");
        break;
    case SUBMODE_f:
        this->modeLabel->setText("f");
        break;
    case SUBMODE_F:
        this->modeLabel->setText("F");
        break;
    case SUBMODE_d:
        this->modeLabel->setText("d");
        break;
    case SUBMODE_y:
        this->modeLabel->setText("y");
        break;
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
    // note that the findBlockByLineNumber starts with 0
    QTextBlock block = this->document()->findBlockByLineNumber(lineNumber - 1);
    QTextCursor cursor = this->textCursor();
    cursor.setPosition(block.position());
    this->setTextCursor(cursor);
}

void Editor::goToOccurrence(const QString& string, bool backward) {
    QString s = string;
    QSettings settings("mehteor", "meh");
    if (string == "") {
        s = settings.value("editor/last_value_go_to_occurrence", "").toString();
    } else {
        settings.setValue("editor/last_value_go_to_occurrence", string);
    }

    if (backward) {
        this->find(s, QTextDocument::FindBackward);
    } else {
        if (!this->find(s)) {
            // didn't find anything, look backward
            this->find(s, QTextDocument::FindBackward);
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

void Editor::paintEvent(QPaintEvent* event) {
    Q_ASSERT(event != NULL);

    QTextEdit::paintEvent(event);
    QPainter painter(this->viewport());
    painter.setPen(QPen(QColor(255, 255, 255, 8)));
    painter.drawLine(this->eightyCharsX, 0, eightyCharsX, this->viewport()->rect().height());
}

void Editor::mousePressEvent(QMouseEvent* event) {
    Q_ASSERT(event != NULL);

    if (event->button() == Qt::MidButton) {
        if (this->mode == MODE_NORMAL) {
            this->setMode(MODE_INSERT);
        } else if (this->mode == MODE_INSERT) {
            this->setMode(MODE_NORMAL);
        }
        return;
    }

    QTextEdit::mousePressEvent(event);
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
                    QTextEdit::keyPressEvent(&pageEvent);
                }
                return;
            case Qt::Key_D:
                {
                    QKeyEvent pageEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
                    QTextEdit::keyPressEvent(&pageEvent);
                }
                return;
            case Qt::Key_Space:
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

            // switch to the other buffer
            case Qt::Key_O:
                {
                    if (this->buffersPos.size() == 0) {
                        return;
                    }
                    // NOTE(remy): don't remove it here, just take a ref,
                    // the selectBuffer takes care of the list order etc.
                    const QString& filename = this->buffersPos.last();
                    this->selectOrCreateBuffer(filename);
                }
                return;

            // open a file with the FilesLookup
            case Qt::Key_P:
                {
                    this->window->openList();
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

    // close extra widgets
    if (event->key() == Qt::Key_Escape) {
        this->window->closeList();
        this->window->closeGrep();
        this->window->closeCompleter();
    }

    // Replace mode
    // ----------------------

    if (this->mode == MODE_REPLACE || this->mode == MODE_REPLACE_ONE) {
        if (event->key() == Qt::Key_Escape) {
            this->setMode(MODE_NORMAL);
            return;
        }

        QTextEdit::keyPressEvent(event);
        if (this->mode == MODE_REPLACE_ONE && event->text()[0] != '\x0') {
            this->setMode(MODE_NORMAL);
            this->left();
        }
        return;
    }

    // Visual mode
    // ----------------------

    if (this->mode == MODE_VISUAL) {
        this->keyPressEventVisual(event, ctrl, shift);
        return;
    }

    if (this->mode == MODE_VISUAL_LINE) {
        this->keyPressEventVisualLine(event, ctrl, shift);
        return;
    }

    // Normal mode
    // ----------------------

    if (this->mode == MODE_NORMAL) {
        this->keyPressEventNormal(event, ctrl, shift);
        return;
    }

    // Insert mode
    // ----------------------

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
            QChar lastChar = this->currentLineLastChar(false);
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
        QTextEdit::keyPressEvent(event);
        return;
    }

    // Otherwise, rely on the original behavior of QTextEdit
    QTextEdit::keyPressEvent(event);
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

QChar Editor::currentLineLastChar(bool moveUp) {
    QTextCursor cursor = this->textCursor();
    if (moveUp) {
        cursor.movePosition(QTextCursor::Up);
    }
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

    this->selectionTimer->stop();
    this->window->openCompleter(base, list);
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

    for (int i = cursor.positionInBlock(); i >= 0; i--) {
        if (text[i] == c) {
            return cursor.positionInBlock() - i;
        }
    }

    return 0;
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
