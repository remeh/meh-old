#include <QCoreApplication>
#include <QMessageBox>

#include "command.h"
#include "window.h"

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
            return;
        case Qt::Key_Return:
            this->execute(this->text());
            this->window->closeCommand();
            return;
    }

    QLineEdit::keyPressEvent(event);
}

bool Command::warningModifiedBuffers() {
	QStringList modifiedBuffers = this->window->getEditor()->modifiedBuffers();
	if (modifiedBuffers.size() > 0) {
		QString msg = "Some opened buffers have not been saved:\n\n";
		for (int i = 0; i < modifiedBuffers.size(); i++) {
			msg += modifiedBuffers.at(i) + "\n";
		}
		QMessageBox::warning(this->window, "Unsaved buffers", msg);
		return true;
	}
	return false;
}

bool Command::areYouSure() {
    Q_ASSERT(this->window != nullptr);
    Q_ASSERT(this->window->getEditor() != nullptr);

    if (this->window->getEditor()->getCurrentBuffer() == nullptr) {
        return true;
    }

    if (this->window->getEditor()->getCurrentBuffer()->getFilename().endsWith(".git/COMMIT_EDITMSG")) {
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText("Are you sure to close the app?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec() == QMessageBox::Yes;
}

void Command::execute(QString text) {
    this->clear();

    QStringList list = text.split(" ");
    const QString& command = list[0];

    // quit
    // ----------------------

    if (command == ":q" || command == ":qa") {
		if (this->warningModifiedBuffers()) {
			return;
		}
        if (this->areYouSure()) {
            QCoreApplication::quit();
        }
        return;
    }

    if (command == ":q!" || command == ":qa!") {
        if (this->areYouSure()) {
            QCoreApplication::quit();
        }
        return;
    }

    if (command == ":x" || command == ":x!") {
        if (list.size() > 1) {
            // TODO(remy):  save to another file
        }
        this->window->getEditor()->save();
		if (this->warningModifiedBuffers()) {
			return;
		}
        if (this->areYouSure()) {
            QCoreApplication::quit();
        }
        return;
    }

    if (command == ":xa" || command == ":xa!") {
        if (list.size() > 1) {
            // TODO(remy):  save to another file
        }
        this->window->getEditor()->saveAll();
        if (this->areYouSure()) {
            QCoreApplication::quit();
        }
        return;
    }

    if (command == ":bd") {
        if (this->window->getEditor()->getCurrentBuffer() == nullptr) {
            return;
        }
        if (this->window->getEditor()->getCurrentBuffer()->modified) {
            QMessageBox::warning(this->window, "Unsaved buffer", "The buffer you want to close has modifications.");
            return;
        }
        this->window->getEditor()->closeCurrentBuffer();
        return;
    }

    if (command == ":bd!") {
        if (this->window->getEditor()->getCurrentBuffer() == nullptr) {
            return;
        }
        this->window->getEditor()->closeCurrentBuffer();
        return;
    }

    if (command == ":basedir") {
        this->window->setBaseDir(list[1]);
        return;
    }

    // go to a specific line
    // ----------------------

    if (command.size() > 1 && command.at(0) == ":" && command.at(1).isDigit()) {
        QStringRef lineStr = command.rightRef(command.size() - 1);
        bool ok = true;
        int line = lineStr.toInt(&ok);
        if (ok) {
            this->window->getEditor()->goToLine(line);
        }
        return;
    }

    // search next occurrence of a string
    // ----------------------

    if (command.size() > 1 && command.at(0) == "/") {
        QStringList search;
        QStringRef first = command.rightRef(command.size() - 1);
        search << first.toString();
        for (int i = 1; i < list.size(); i++) {
           search << list.at(i);
        }
        this->window->getEditor()->goToOccurrence(search.join(" "), false);
        return;
    }

    // grep
    // ----------------------

    if (command.startsWith(":rg")) {
        QString search = "";

        if (list.size() > 1) {
            QStringList listCopy = list;
            listCopy.removeFirst();
            search = listCopy.join(" ");
        } else {
            search = this->window->getEditor()->getWordUnderCursor();
        }

        if (command.startsWith(":rgf")) {
            this->window->openGrep(search, this->window->getEditor()->getCurrentBuffer()->getFilename());
        } else {
            this->window->openGrep(search);
        }

        return;
    }

    // open a file
    // ----------------------

    if (command == ":e" && list.size() > 1) {
        for (int i = 1; i < list.size(); i++) {
            const QString& file = list[i];
            this->openFile(file);
        }
        return;
    }

    // save
    // ----------------------

    if (command == ":w") {
        if (list.size() > 1) {
            // TODO(remy): save to another file
        }
        this->window->getEditor()->save();
    }

    if (command == ":wa") {
        this->window->getEditor()->saveAll();
    }
}

void Command::openFile(const QString& filename) {
    Editor* editor = this->window->getEditor();
    if (editor == nullptr) {
        // TODO(remy): display a warning
        return;
    }

    // if current buffer, don't do anything
    if (editor->getCurrentBuffer() != nullptr &&
        editor->getCurrentBuffer()->getFilename() == filename) {
        return;
    }

    // this will automatically creates a buffer if needed.
    editor->selectOrCreateBuffer(filename);
}
