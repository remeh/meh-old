#include <QColor>
#include <QKeyEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
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
    window(window),
    currentBuffer(NULL),
    mode(MODE_NORMAL),
    highlightedLine(QColor::fromRgb(40, 40, 40)) {
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

    this->lineLabel = new QLabel("  1", this);
    this->lineLabel->setStyleSheet("color: rgba(255, 255, 255, 30); background-color: rgba(0, 0, 0, 0);");
    this->lineLabel->setFont(labelFont);
    this->lineLabel->show();

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
}

void Editor::onWindowResized(QResizeEvent*) {
    int x = this->window->rect().width() - 90;
    this->modeLabel->move(x, 2);
    this->lineLabel->move(x, 20);
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

void Editor::setCurrentBuffer(Buffer* buffer) {
    Q_ASSERT(buffer != NULL);

    if (this->currentBuffer != NULL) {
        this->currentBuffer->onLeave(this);
        // we're leaving this one, append it to the end of the buffers list.
        this->buffersPos.append(this->currentBuffer->getFilename());
        this->buffers[this->currentBuffer->getFilename()] = this->currentBuffer;
    }

    this->currentBuffer = buffer;
    this->currentBuffer->onEnter(this);
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
        }
    }

    // close extra widgets
    if (event->key() == Qt::Key_Escape) {
        this->window->closeList();
        this->window->closeGrep();
    }

    // Replace mode
    // ----------------------

    if (this->mode == MODE_REPLACE) {
        if (event->key() == Qt::Key_Escape) {
            this->setMode(MODE_NORMAL);
            return;
        }

        QTextEdit::keyPressEvent(event);
        return;
    }

    // Visual mode
    // ----------------------

    if (this->mode == MODE_VISUAL) {
        this->keyPressEventVisual(event, ctrl, shift);
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
        }

        this->insertPlainText("\n" + indent);
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
            this->removeIndentation();
        }
        QTextEdit::keyPressEvent(event);
        return;
    }

    // Otherwise, rely on the original behavior of QTextEdit
    QTextEdit::keyPressEvent(event);
}

void Editor::removeIndentation() {
    // remove one layer of indentation
    QTextCursor cursor = this->textCursor();
    int position = cursor.position();
    cursor.movePosition(QTextCursor::StartOfLine);
    int i = 0;
    for (i = 0; i < 4; i++) {
        if (this->document()->characterAt(cursor.position()) == " ") {
            cursor.deleteChar();
        } else {
            break;
        }
    }
    cursor.setPosition(position - i);
    this->setTextCursor(cursor);
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

QString Editor::getWordUnderCursor() {
    QString rv;

    QTextCursor cursor = this->textCursor();
    QString text = cursor.block().text();

    int pos = cursor.positionInBlock();
    if (text[pos].isNumber() || text[pos].isLetter() || text[pos] == '_') {
        rv.append(text[pos]);
    } else {
        return "";
    }

    for (int i = pos-1; i >= 0; i--) {
        if (text[i].isNumber() || text[i].isLetter() || text[i] == '_') {
            rv.prepend(text[i]);
            continue;
        }
        break;
    }
    for (int i = pos+1; i <= text.size(); i++) {
        if (text[i].isNumber() || text[i].isLetter() || text[i] == '_') {
            rv.append(text[i]);
            continue;
        }
        break;
    }

    return rv;
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
