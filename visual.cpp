#include <QChar>
#include <QKeyEvent>
#include <QTextCursor>

#include "editor.h"
#include "window.h"

void Editor::keyPressEventVisual(QKeyEvent* event, bool ctrl, bool shift) {
    Q_ASSERT(event != NULL);

    if (this->subMode == SUBMODE_f || this->subMode == SUBMODE_F) {
        this->keyPressEventSubMode(event, ctrl, shift);
        return;
    }

    switch (event->key()) {
        case Qt::Key_Escape:
            {
                this->setMode(MODE_NORMAL);
                QTextCursor cursor = this->textCursor();
                cursor.clearSelection();
                this->setTextCursor(cursor);
            }
            break;
        case Qt::Key_F:
            if (shift) {
                this->setSubMode(SUBMODE_F);
            } else {
                this->setSubMode(SUBMODE_f);
            }
            return;
        case Qt::Key_Dollar:
            this->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            return;
        case Qt::Key_Y:
            {
                this->copy();
                QTextCursor cursor = this->textCursor();
                cursor.clearSelection();
                this->setTextCursor(cursor);
                this->setMode(MODE_NORMAL);
            }
            return;
        case Qt::Key_X:
            this->cut();
            this->setMode(MODE_NORMAL);
            return;

        case Qt::Key_K:
            this->moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
            return;
        case Qt::Key_J:
            if (!shift) {
                this->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
            }
            return;
        case Qt::Key_Backspace:
        case Qt::Key_H:
            this->moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
            return;
        case Qt::Key_L:
            {
                QTextCursor cursor = this->textCursor();
                QChar c = this->document()->characterAt(cursor.position()+1);
                if (c != u'\u2029') {
                    this->moveCursor(QTextCursor::Right, QTextCursor::KeepAnchor);
                }
            }
            return;
        case Qt::Key_G:
            if (shift) {
                this->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
            } else {
                this->moveCursor(QTextCursor::Start, QTextCursor::KeepAnchor);
            }
            return;

        case Qt::Key_E:
            if (shift) { /* TODO(remy): move right until previous space */ }
            // TODO(remy): confirm this is the wanted behavior
            this->moveCursor(QTextCursor::NextWord, QTextCursor::KeepAnchor);
            return;
        case Qt::Key_B:
            if (shift) { /* TODO(remy): move left until previous space */ }
            // TODO(remy): confirm this is the wanted behavior
            this->moveCursor(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            return;
        case Qt::Key_Less:
            {
                this->textCursor().beginEditBlock();
                QList<QTextBlock> blocks = this->selectedBlocks();
                for (int i = 0; i < blocks.size(); i++) {
                    this->removeIndentation(QTextCursor(blocks.at(i)));
                }
                this->textCursor().endEditBlock();
            }
            return;
        case Qt::Key_Greater:
            {
                this->textCursor().beginEditBlock();
                QList<QTextBlock> blocks = this->selectedBlocks();
                for (int i = 0; i < blocks.size(); i++) {
                    this->insertIndentation(QTextCursor(blocks.at(i)));
                }
                this->textCursor().endEditBlock();
            }
            return;
        }
}


void Editor::keyPressEventVisualLine(QKeyEvent* event, bool, bool shift) {
    Q_ASSERT(event != NULL);

    switch (event->key()) {
        case Qt::Key_Escape:
            {
                this->setMode(MODE_NORMAL);
                QTextCursor cursor = this->textCursor();
                cursor.clearSelection();
                this->setTextCursor(cursor);
            }
            break;
        case Qt::Key_Y:
            {
                QChar c = this->document()->characterAt(this->textCursor().position());
                if (c == u'\u2029') {
                    QTextCursor cursor = this->textCursor();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    this->setTextCursor(cursor);
                }
                this->copy();
                QTextCursor cursor = this->textCursor();
                cursor.clearSelection();
                this->setTextCursor(cursor);
                this->setMode(MODE_NORMAL);
            }
            return;
        case Qt::Key_D:
        case Qt::Key_X:
            {
                QChar c = this->document()->characterAt(this->textCursor().position());
                if (c == u'\u2029') {
                    QTextCursor cursor = this->textCursor();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    this->setTextCursor(cursor);
                }
                this->cut();
                this->setMode(MODE_NORMAL);
            }
            return;
        case Qt::Key_K:
            this->moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
            this->selectVisualLineSelection();
            return;
        case Qt::Key_J:
            this->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
            this->selectVisualLineSelection();
            return;
        case Qt::Key_G:
            if (shift) {
                this->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
            } else {
                this->moveCursor(QTextCursor::Start, QTextCursor::KeepAnchor);
            }
            return;
        case Qt::Key_Less:
        case Qt::Key_Greater:
            {
                this->textCursor().beginEditBlock();
                QList<QTextBlock> blocks = this->selectedBlocks();
                int blocksSize = blocks.size();
                // if we're going up, we don't want to consider the last line
                if (this->textCursor().blockNumber() < this->visualLineBlockStart.blockNumber()) {
                    blocksSize--;
                }
                for (int i = 0; i < blocksSize; i++) {
                    if (event->key() == Qt::Key_Less) {
                        this->removeIndentation(QTextCursor(blocks.at(i)));
                    } else {
                        this->insertIndentation(QTextCursor(blocks.at(i)));
                    }
                }
                this->textCursor().endEditBlock();
            }
            return;
    }
}

void Editor::selectVisualLineSelection() {
    QTextCursor cursor = this->textCursor();
    if (this->visualLineBlockStart.blockNumber() == cursor.blockNumber()) {
        cursor.setPosition(this->visualLineBlockStart.position());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        return;
    } else if (cursor.blockNumber() < this->visualLineBlockStart.blockNumber()) {
        int target = cursor.block().position();
        cursor.setPosition(this->visualLineBlockStart.position()+this->visualLineBlockStart.length(), QTextCursor::MoveAnchor);
        cursor.setPosition(target, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        return;
    } else if (cursor.blockNumber() > this->visualLineBlockStart.blockNumber()) {
        int target = cursor.block().position() + cursor.block().length() - 1;
        cursor.setPosition(this->visualLineBlockStart.position(), QTextCursor::MoveAnchor);
        cursor.setPosition(target, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        return;
    }
}