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
			this->clear();
			this->window->closeCommand();
			break;
		case Qt::Key_Return:
			this->execute(this->text());
			this->window->closeCommand();
			break;
	}

	QLineEdit::keyPressEvent(event);
}

void Command::execute(QString text) {
	this->clear();

	if (text == ":q!") {
		QCoreApplication::quit();
		return;
	}

	// go to a specific line
	if (text.size() > 1 && text.at(0) == ":" && text.at(1).isDigit()) {
		QStringRef lineStr = text.rightRef(text.size() - 1);
		bool ok = true;
		int line = lineStr.toInt(&ok);
		if (ok) {
			this->window->getEditor()->goToLine(line);
		}
		return;
	}
}
