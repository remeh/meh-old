#include <QCoreApplication>

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

void Command::execute(QString text) {
    this->clear();

    QStringList list = text.split(" ");
    const QString& command = list[0];

    // quit
    // ----------------------

    if (command == ":q!" || command == ":qa!") {
        QCoreApplication::quit();
        return;
    }

    if (command == ":x" || command == ":x!") {
        if (list.size() > 1) {
            // TODO(remy):  save to another file
        }
        this->window->getEditor()->save();
        QCoreApplication::quit();
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
        QStringRef search = command.rightRef(command.size() - 1);
        this->window->getEditor()->goToOccurrence(search.toString(), false);
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

    // save a file
    if (command == ":w") {
        if (list.size() > 1) {
            // TODO(remy):  save to another file
        }
        this->window->getEditor()->save();
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
