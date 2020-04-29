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
				if (c != "\u2029") {
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
	}
}
