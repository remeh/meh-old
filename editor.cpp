#include <QCoreApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
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
	window(window),
	currentBuffer(NULL),
	mode(MODE_NORMAL) {
	Q_ASSERT(window != NULL);

	// we don't want a rich text editor
	// ----------------------

	this->setAcceptRichText(false);

	// start in normal mode
	// ----------------------

	this->setMode(MODE_NORMAL);
	this->setSubMode(NO_SUBMODE);

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

	// tab space size
	// ----------------------

	const int tabSpace = 4;
	QFontMetrics metrics(font);
	this->setTabStopDistance(tabSpace*metrics.horizontalAdvance(" "));

	connect(this, &QTextEdit::selectionChanged, this, &Editor::onSelectionChanged);
	connect(this->selectionTimer, &QTimer::timeout, this, &Editor::onTriggerSelectionHighlight);
}

Editor::~Editor() {
	// TODO(remy): delete the buffers
	delete this->selectionTimer;
	this->selectionTimer = nullptr;
}

void Editor::onSelectionChanged() {
	this->selectionTimer->start(500);
}

void Editor::onTriggerSelectionHighlight() {
	QTextCursor cursor = this->textCursor();
	if (this->syntax->setSelection(cursor.selectedText())) {
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
	// NOTE(remy): here we could run a leaveMode(mode) method?
	this->setOverwriteMode(false);
	switch (mode) {
	case MODE_NORMAL:
	default:
		this->setBlockCursor();
		break;
	case MODE_VISUAL:
		this->setMidCursor();
		break;
	case MODE_INSERT:
		this->setLineCursor();
		break;
	case MODE_COMMAND:
		this->setBlockCursor();
		this->window->openCommand();
		if (command.size() > 0) {
			this->window->setCommand(command);
		}
		break;
	case MODE_REPLACE:
		this->setMidCursor();
		this->setOverwriteMode(true);
		break;
	}
	this->mode = mode;
}

void Editor::setSubMode(int subMode) {
	if (subMode != NO_SUBMODE) {
		this->setMidCursor();
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

void Editor::deleteCurrentLine() {
	QTextCursor cursor = this->textCursor();
	cursor.movePosition(QTextCursor::StartOfBlock);
	cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();
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

			// switch to previous buffer
			case Qt::Key_P:
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
			case Qt::Key_O:
				{
					this->window->openList();
				}
				return;
		}
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
		}
		this->insertPlainText("\n" + indent);
		return;
	}

	// Otherwise, rely on the original behavior of QTextEdit
	QTextEdit::keyPressEvent(event);
}

// TODO(remy): I should consider moving this to its own class or at least its own file
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
		case Qt::Key_Colon:
			this->setMode(MODE_COMMAND, ":");
			break;
		case Qt::Key_W:
			this->setMode(MODE_COMMAND, ":w");
			return;

		case Qt::Key_Y:
			if (!shift) {
				this->setSubMode(SUBMODE_y);
			}
			return;

		case Qt::Key_V:
			if (shift) {
				// TODO(remy): implement me
			}
			this->setMode(MODE_VISUAL);
			return;

		case Qt::Key_R:
			this->setMode(MODE_REPLACE);
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

		case Qt::Key_O:
			// NOTE(remy): we could detect the { at the end of a line
			// to have another behavior.
			if (shift) {
				this->moveCursor(QTextCursor::Up);
			}
			this->moveCursor(QTextCursor::EndOfLine);
			this->insertPlainText("\n" + this->currentLineIndent());
			this->setMode(MODE_INSERT);
			return;

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
			this->moveCursor(QTextCursor::Up);
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
				this->moveCursor(QTextCursor::Down);
			}
			return;
		case Qt::Key_Backspace:
		case Qt::Key_H:
			this->moveCursor(QTextCursor::Left);
			return;
		case Qt::Key_L:
			{
				QTextCursor cursor = this->textCursor();
				QChar c = this->document()->characterAt(cursor.position()+1);
				if (c != "\u2029") {
					this->moveCursor(QTextCursor::Right);
				}
			}
			return;

		case Qt::Key_P:
			{
				QTextCursor cursor = this->textCursor();
				cursor.beginEditBlock();
				if (shift) {
					this->moveCursor(QTextCursor::StartOfBlock);
					this->paste();
					this->moveCursor(QTextCursor::Left);
				} else {
					this->moveCursor(QTextCursor::Down);
					this->moveCursor(QTextCursor::StartOfBlock);
					this->paste();
					this->moveCursor(QTextCursor::Left);
				}
				cursor.endEditBlock();
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
				// TODO(remy): remove the end of line but stay in normal mode
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
	}
}


// TODO(remy): I should consider moving this to its own class or at least its own file
void Editor::keyPressEventVisual(QKeyEvent* event, bool ctrl, bool shift) {
	Q_ASSERT(event != NULL);

	if (this->subMode == SUBMODE_f || this->subMode == SUBMODE_F) {
		this->keyPressEventSubMode(event, ctrl, shift);
		return;
	}

	switch (event->key()) {
		case Qt::Key_Escape:
			this->setMode(MODE_NORMAL);
			this->textCursor().clearSelection();
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
