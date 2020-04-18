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

#include "editor.h"
#include "mode.h"
#include "window.h"

Editor::Editor(Window* window) :
	window(window),
	currentBuffer(NULL),
	mode(MODE_NORMAL) {
	Q_ASSERT(window != NULL);

	// we don't want a rich text editor
	this->setAcceptRichText(false);
	this->setMode(MODE_NORMAL);
	this->setSubMode(NO_SUBMODE);

	// editor font
	// ----------------------

	QFont font;
	font.setFamily("Inconsolata");
	font.setStyleHint(QFont::Monospace);
	font.setFixedPitch(true);
	#ifdef Q_OS_MAC
	font.setPointSize(12);
	#else
	font.setPointSize(10);
	#endif
	this->setFont(font);

	// tab space size

	const int tabSpace = 4;
	QFontMetrics metrics(font);
	this->setTabStopDistance(tabSpace*metrics.horizontalAdvance(" "));
}

Editor::~Editor() {
	// TODO(remy): delete the buffers
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

void Editor::selectBuffer(const QString& filename) {
	QFileInfo info(filename);
	Buffer* buffer = this->buffers.take(info.absoluteFilePath());
	this->buffersPos.remove(this->buffersPos.indexOf(info.absoluteFilePath()));
	if (buffer == nullptr) {
		// TODO(remy): ???
		return;
	}
	this->setCurrentBuffer(buffer);
}

bool Editor::hasBuffer(const QString& filename) {
	QFileInfo info(filename);
	return this->buffers.contains(info.absoluteFilePath());
}

void Editor::setMode(int mode, QString command) {
	// NOTE(remy): here we could run a leaveMode(mode) method?
	this->setOverwriteMode(false);
	switch (mode) {
	case MODE_NORMAL:
	default:
		this->setBlockCursor();
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
		this->setBlockCursor();
		this->setOverwriteMode(true);
		break;
	}
	this->mode = mode;
}

void Editor::setSubMode(int subMode) {
	// TODO(remy): change the cursor
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

	// Normal mode
	// ----------------------

	if (this->mode == MODE_NORMAL) {
		this->keyPressEventNormal(event, ctrl, shift);
		return;
	}

	// Insert mode
	// ----------------------

	if (event->key() == Qt::Key_Escape) {
		this->setMode(MODE_NORMAL);
		return;
	}

	// Otherwise, rely on the original behavior of QTextEdit
	QTextEdit::keyPressEvent(event);
}

void Editor::keyPressEventNormal(QKeyEvent* event, bool ctrl, bool shift) {
	Q_ASSERT(event != NULL);

	if (this->subMode != NO_SUBMODE) {
		this->keyPressEventSubMode(event, ctrl, shift);
		return;
	}

	switch (event->key()) {
		case Qt::Key_Escape:
			this->setMode(MODE_COMMAND);
			break;
		case Qt::Key_Colon:
			this->setMode(MODE_COMMAND, ":");
			break;
		case Qt::Key_W:
			this->setMode(MODE_COMMAND, ":w");
			return;

		case Qt::Key_R:
			this->setMode(MODE_REPLACE);
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
			this->textCursor().deleteChar();
			return;

		case Qt::Key_K:
			this->moveCursor(QTextCursor::Up);
			return;
		case Qt::Key_J:
			this->moveCursor(QTextCursor::Down);
			return;
		case Qt::Key_H:
			this->moveCursor(QTextCursor::Left);
			return;
		case Qt::Key_L:
			this->moveCursor(QTextCursor::Right);
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
				// TODO(remy): remove the end of the line and enter insert mode
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

void Editor::keyPressEventSubMode(QKeyEvent* event, bool ctrl, bool shift) {
	if (event->key() ==  Qt::Key_Escape) {
		this->setSubMode(NO_SUBMODE);
		this->setMode(MODE_NORMAL);
		return;
	}

	switch (this->subMode) {
		case SUBMODE_c:
			if (!shift && event->key() == Qt::Key_C) {
				QString indent = this->currentLineIndent();
				this->deleteCurrentLine();
				this->insertPlainText(indent);
				this->setMode(MODE_INSERT);
				this->setSubMode(NO_SUBMODE);
			return;
			}
			break;
		case SUBMODE_cf:
		case SUBMODE_cF:
		case SUBMODE_f:
		case SUBMODE_F:
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

	this->setMode(MODE_NORMAL);
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

