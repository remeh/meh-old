#include <QCoreApplication>
#include <QDebug>
#include <QKeyEvent>
#include <qglobal.h>
#include <QPainter>
#include <QTextEdit>
#include <QTextCursor>

#include "window.h"
#include "buffer.h"

Window::Window(QWidget *parent) :
	QTextEdit(parent),
	currentBuffer(NULL) {
	// we don't want a rich text editor
	this->setAcceptRichText(false);
}

void Window::setCurrentBuffer(Buffer* buffer) {
	Q_ASSERT(buffer != NULL);

	this->currentBuffer = buffer;

	// load the content of the buffer
	this->setText(this->currentBuffer->readFile());

	// TODO(remy): set the cursor position to the latest used
}

void Window::keyPressEvent(QKeyEvent *event) {
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
