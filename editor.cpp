#include <QCoreApplication>
#include <QDebug>
#include <QKeyEvent>
#include <qglobal.h>
#include <QPainter>
#include <QTextEdit>
#include <QTextCursor>

#include "buffer.h"
#include "editor.h"
#include "mode.h"

Editor::Editor(QWidget *parent) :
	QTextEdit(parent),
	currentBuffer(NULL),
	mode(MODE_INSERT) {
	// we don't want a rich text editor
	this->setAcceptRichText(false);
}

void Editor::setCurrentBuffer(Buffer* buffer) {
	Q_ASSERT(buffer != NULL);

	if (this->currentBuffer != NULL) {
		this->currentBuffer->onLeave(this);
	}

	this->currentBuffer = buffer;
	this->currentBuffer->onEnter(this);
}

void Editor::keyPressEvent(QKeyEvent *event) {
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

	if (ctrl) {
		switch (event->key()) {
			case Qt::Key_U:
				{
					QKeyEvent *pageEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier);
					QCoreApplication::postEvent(this, pageEvent);
				}
				return;
			case Qt::Key_D:
				{
					QKeyEvent *pageEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
					QCoreApplication::postEvent(this, pageEvent);
				}
				return;
		}
	} else {
		// TODO(remy): depend on the active mode
		switch (event->key()) {
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
			case Qt::Key_Escape:
				this->setBlockCursor();
				return;
			case Qt::Key_I:
				this->setLineCursor();
				return;
		}
	}

	// otherwise, rely on the original behavior of QTextEdit
	QTextEdit::keyPressEvent(event);
}
