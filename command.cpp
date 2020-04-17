#include <QCoreApplication>

#include "command.h"
#include "window.h"

#include "qdebug.h"

Command::Command(Window* window) :
	window(window) {
	Q_ASSERT(window != NULL);
}

void Command::keyPressEvent(QKeyEvent* event) {
	Q_ASSERT(event != NULL);

	switch (event->key()) {
		case Qt::Key_Escape:
			// return to normal mode
			this->clear();
			this->window->closeCommand();
			break;
		case Qt::Key_Return:
			// return to normal mode
			this->execute();
			this->window->closeCommand();
			break;
	}

	QLineEdit::keyPressEvent(event);
}

void Command::execute() {
	// TODO(remy): implement me.
	if (this->text() == ":q!") {
		QCoreApplication::quit();
		return;
	}

	this->clear();
}
