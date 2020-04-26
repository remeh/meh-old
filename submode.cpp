#include <QChar>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>

#include "editor.h"

void Editor::keyPressEventSubMode(QKeyEvent* event, bool, bool shift) {
	if (event->key() ==  Qt::Key_Escape) {
		this->setSubMode(NO_SUBMODE);
		this->setMode(MODE_NORMAL);
		return;
	}

	// ignore modifiers keys, etc.
	// NOTE(remy): maybe we will have to do that only for some submodes
	if (event->text()[0] == '\x0') {
		return;
	}

	switch (this->subMode) {
		case SUBMODE_c:
			switch (event->key()) {
				case Qt::Key_C:
					if (!shift) {
						QTextCursor cursor = this->textCursor();
						QString indent = this->currentLineIndent();
						cursor.beginEditBlock();
						this->deleteCurrentLine();
						this->insertPlainText(indent);
						cursor.endEditBlock();
						this->setMode(MODE_INSERT);
						this->setSubMode(NO_SUBMODE);
						return;
					}
					break;
				case Qt::Key_F:
					if (shift) {
						this->setSubMode(SUBMODE_cF);
						return;
					} else {
						this->setSubMode(SUBMODE_cf);
						return;
					}
				case Qt::Key_T:
					if (shift) {
						this->setSubMode(SUBMODE_cT);
						return;
					} else {
						this->setSubMode(SUBMODE_ct);
						return;
					}
			}
			break;
		case SUBMODE_cf:
		case SUBMODE_cF:
		case SUBMODE_ct:
		case SUBMODE_cT:
			{
				int distance = 0;
				if (this->subMode == SUBMODE_cf || this->subMode == SUBMODE_ct) {
					distance = this->findNextOneInCurrentLine(event->text()[0]);
				} else {
					distance = this->findPreviousOneInCurrentLine(event->text()[0]);
				}

				if (distance == 0) {
					// nothing has been found
					break;
				}

				if (this->subMode == SUBMODE_cf) {
					distance += 1;
				} else if (this->subMode == SUBMODE_cT) {
					distance -= 1;
				}

				QTextCursor cursor = this->textCursor();
				cursor.beginEditBlock();
				if (this->subMode == SUBMODE_cf || this->subMode == SUBMODE_ct) {
					cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, distance);
				} else {
					cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, distance);
				}
				this->setTextCursor(cursor);
				this->cut();
				cursor.endEditBlock();
			}
			this->setMode(MODE_INSERT);
			this->setSubMode(NO_SUBMODE);
			return;
		case SUBMODE_y:
			if (!shift && event->key() == Qt::Key_Y) {
				QTextCursor cursor = this->textCursor();
				int position = cursor.position();
				cursor.movePosition(QTextCursor::StartOfBlock);
				cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
				cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
				this->setTextCursor(cursor);
				this->copy();
				cursor.clearSelection();
				cursor.setPosition(position);
				this->setTextCursor(cursor);
			}
			break;

		case SUBMODE_f:
			{
				int distance = this->findNextOneInCurrentLine(event->text()[0]);
				QTextCursor cursor = this->textCursor();
				auto move = QTextCursor::MoveAnchor;
				if (this->mode == MODE_VISUAL) {
					move = QTextCursor::KeepAnchor;
					distance += 1;
				}
				cursor.movePosition(QTextCursor::Right, move, distance);
				this->setTextCursor(cursor);
			}
			break;
		case SUBMODE_F:
			{
				int distance = this->findPreviousOneInCurrentLine(event->text()[0]);
				QTextCursor cursor = this->textCursor();
				auto move = QTextCursor::MoveAnchor;
				if (this->mode == MODE_VISUAL) { move = QTextCursor::KeepAnchor; }
				cursor.movePosition(QTextCursor::Left, move, distance);
				this->setTextCursor(cursor);
			}
			break;
		case SUBMODE_d:
			if (!shift && event->key() == Qt::Key_D) {
				QTextCursor cursor = this->textCursor();
				cursor.movePosition(QTextCursor::StartOfBlock);
				cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
				cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
				this->setTextCursor(cursor);
				this->cut();
			}
			break;
	}

	if (this->mode != MODE_VISUAL) {
		this->setMode(MODE_NORMAL);
	}
	this->setSubMode(NO_SUBMODE);
}
