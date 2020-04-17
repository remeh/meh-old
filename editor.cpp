#include <QCoreApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QTextEdit>
#include <QTextCursor>

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
	this->setTabStopDistance(tabSpace*metrics.averageCharWidth());
}

Editor::~Editor() {
	// TODO(remy): delete the buffers
}

void Editor::setCurrentBuffer(Buffer* buffer) {
	Q_ASSERT(buffer != NULL);

	if (this->currentBuffer != NULL) {
		this->currentBuffer->onLeave(this);
	}

	this->currentBuffer = buffer;
	this->currentBuffer->onEnter(this);
}

void Editor::setMode(int mode) {
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
		break;
	}
	this->mode = mode;
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

	switch (event->key()) {
		case Qt::Key_Escape:
			this->setMode(MODE_COMMAND);
			break;

		case Qt::Key_I:
			if (shift) {
				this->moveCursor(QTextCursor::StartOfLine);
			}
			this->setMode(MODE_INSERT);
			return;
		case Qt::Key_A:
			if (shift) {
				this->moveCursor(QTextCursor::EndOfLine);
			}
			this->setMode(MODE_INSERT);
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
