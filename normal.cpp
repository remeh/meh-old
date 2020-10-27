#include <QChar>
#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include "qdebug.h"

#include "editor.h"
#include "window.h"

void Editor::keyPressEventNormal(QKeyEvent* event, bool ctrl, bool shift) {
    Q_ASSERT(event != NULL);

    if (this->subMode != NO_SUBMODE) {
        this->keyPressEventSubMode(event, ctrl, shift);
        return;
    }

    switch (event->key()) {
        case Qt::Key_Escape:
            this->setMode(MODE_NORMAL);
            break;
        case Qt::Key_Slash:
            this->setMode(MODE_COMMAND, "/");
            break;
        case Qt::Key_Colon:
            this->setMode(MODE_COMMAND, ":");
            break;
        case Qt::Key_W:
            this->setMode(MODE_COMMAND, ":w");
            return;

        // arrow keys in normal mode
        case Qt::Key_Left:
            this->left();
            return;
        case Qt::Key_Down:
            this->down();
            return;
        case Qt::Key_Up:
            this->up();
            return;
        case Qt::Key_Right:
            this->right();
            return;

        case Qt::Key_Y:
            {
                QTextCursor cursor = this->textCursor();
                if (cursor.hasSelection()) {
                    this->copy();
                    cursor.clearSelection();
                    this->setTextCursor(cursor);
                    this->setMode(MODE_NORMAL);
                    return;
                }
                if (!shift) {
                    this->setSubMode(SUBMODE_y);
                }
            }
            return;

        case Qt::Key_V:
            if (shift) {
                this->setMode(MODE_VISUAL_LINE);
                return;
            }
            this->setMode(MODE_VISUAL);
            return;

        case Qt::Key_R:
            if (shift) {
                this->setMode(MODE_REPLACE);
                return;
            }
            this->setMode(MODE_REPLACE_ONE);
            return;

        case Qt::Key_N:
            if (shift) {
                this->goToOccurrence("", true);
            } else {
                this->goToOccurrence("", false);
            }
            return;

        case Qt::Key_Question:
            this->goToOccurrence(this->getWordUnderCursor(), false);
            return;
        case Qt::Key_Comma:
            this->setMode(MODE_LEADER);
            return;

        case Qt::Key_F:
            if (shift) {
                this->setSubMode(SUBMODE_F);
            } else {
                this->setSubMode(SUBMODE_f);
            }
            return;

        case Qt::Key_I:
            if (shift) {
                this->moveCursor(QTextCursor::StartOfBlock);
            }
            this->setMode(MODE_INSERT);
            return;
        case Qt::Key_A:
            if (shift) {
                this->moveCursor(QTextCursor::EndOfBlock);
            } else {
                this->moveCursor(QTextCursor::Right);
            }
            this->setMode(MODE_INSERT);
            return;

        case Qt::Key_Greater:
            {
                this->insertIndentation(this->textCursor());
            }
            return;
        case Qt::Key_Less:
            {
                this->removeIndentation(this->textCursor());
            }
            return;

        case Qt::Key_O:
            {
                // NOTE(remy): we could detect the { at the end of a line
                // to have another behavior.
                if (shift) {
                    this->insertNewLine(true);
                    return;
                }
                this->insertNewLine(false);
                return;
            }
        case Qt::Key_Dollar:
            this->moveCursor(QTextCursor::EndOfBlock);
            return;

        case Qt::Key_X:
            {
                QTextCursor cursor = this->textCursor();
                QChar c = this->document()->characterAt(cursor.position());
                if (c != "\u2029") {
                    this->textCursor().deleteChar();
                }
            }
            return;
        case Qt::Key_K:
            this->up();
            return;
        case Qt::Key_J:
            if (shift) {
                this->moveCursor(QTextCursor::EndOfLine);
                QTextCursor cursor = this->textCursor();
                QTextDocument* document = this->document();
                cursor.beginEditBlock();
                cursor.deleteChar();
                QChar c = document->characterAt(cursor.position());
                while (c == "\t" || c == " ") {
                    cursor.deleteChar();
                    c = document->characterAt(cursor.position());
                }
                cursor.insertText(" ");
                cursor.endEditBlock();
                return;
            } else {
                this->down();
            }
            return;
        case Qt::Key_Backspace:
        case Qt::Key_H:
            this->left();
            return;
        case Qt::Key_L:
            this->right();
            return;
        case Qt::Key_P:
            {
                QClipboard* clipboard = QGuiApplication::clipboard();
                bool newline = false;
                if (clipboard != nullptr) {
                    newline = clipboard->text().endsWith("\n");
                }
                QTextCursor cursor = this->textCursor();
                QScrollBar* vscroll = this->verticalScrollBar();
                int value = vscroll->value();
                cursor.beginEditBlock();
                if (shift) {
                    if (newline) {
                        this->moveCursor(QTextCursor::StartOfBlock);
                    }
                    this->paste();
                    if (newline) {
                        this->moveCursor(QTextCursor::Left);
                    }
                } else {
                    if (newline) {
                        this->moveCursor(QTextCursor::Down);
                        this->moveCursor(QTextCursor::StartOfBlock);
                    } else {
                        this->moveCursor(QTextCursor::Right);
                    }
                    this->paste();
                    this->moveCursor(QTextCursor::Left);
                }
                cursor.endEditBlock();
                vscroll->setValue(value);
                this->ensureCursorVisible();
            }
            return;

        case Qt::Key_C:
            if (shift) {
                QTextCursor cursor = this->textCursor();
                cursor.beginEditBlock();
                this->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                this->cut();
                cursor.endEditBlock();
                this->setMode(MODE_INSERT);
            } else {
                this->setSubMode(SUBMODE_c);
            }
            return;

        case Qt::Key_D:
            if (shift) {
                QTextCursor cursor = this->textCursor();
                cursor.beginEditBlock();
                this->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                this->cut();
                cursor.endEditBlock();
            } else {
                this->setSubMode(SUBMODE_d);
            }
            return;

        case Qt::Key_G:
            if (shift) {
                this->moveCursor(QTextCursor::End);
            } else {
                this->moveCursor(QTextCursor::Start);
            }
            return;

        case Qt::Key_E:
            if (shift) { /* TODO(remy): move right until previous space */ }
            // TODO(remy): confirm this is the wanted behavior
            this->moveCursor(QTextCursor::NextWord);
            return;
        case Qt::Key_B:
            if (shift) { /* TODO(remy): move left until previous space */ }
            // TODO(remy): confirm this is the wanted behavior
            this->moveCursor(QTextCursor::PreviousWord);
            return;

        case Qt::Key_U:
            if (shift) {
                this->redo();
            } else {
                this->undo();
            }
            return;

       case Qt::Key_S:
           if (shift) {
               this->lastCheckpoint();
           } else {
               this->saveCheckpoint();
           }
           return;
    }
}

