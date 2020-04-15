#include <QKeyEvent>
#include <QPainter>
#include <QTextEdit>
#include <QTextCursor>

#include "window.h"
#include "buffer.h"

Window::Window(QWidget *parent) :
	QTextEdit(parent),
	currentBuffer(NULL) {
}

void Window::setCurrentBuffer(Buffer* buffer) {
	Q_ASSERT(buffer != NULL);

	this->currentBuffer = buffer;
	this->setText(this->currentBuffer->readFile());
}

void Window::keyPressEvent(QKeyEvent *event) {
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

	// otherwise, rely on the original behavior.
	QTextEdit::keyPressEvent(event);
}
