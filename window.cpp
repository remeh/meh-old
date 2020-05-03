#include <QFileInfo>
#include <QString>

#include "command.h"
#include "editor.h"
#include "fileslookup.h"
#include "grep.h"
#include "window.h"

Window::Window(QWidget* parent) :
    QWidget(parent),
    editor(nullptr) {
    // widgets
    // ----------------------
    this->command = new Command(this);
    this->command->hide();
    this->filesLookup = new FilesLookup(this);
    this->filesLookup->hide();

    // editor
    // ----------------------

    this->editor = new Editor(this);

    // set base dir
    // ----------------------

    QFileInfo dir(".");
    this->baseDir = dir.absoluteFilePath();

    // grep instance
    // ----------------------

    this->grep = new Grep(this);
    this->grep->hide();

    // layout
    // ----------------------

    this->layout = new QGridLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->addWidget(this->editor);
    this->layout->addWidget(this->command);
    this->layout->addWidget(this->filesLookup);
    this->layout->addWidget(this->grep);
    this->setLayout(layout);
}

void Window::openCommand() {
    this->command->show();
    this->command->setFocus();
}

void Window::closeCommand() {
    this->command->hide();
    if (this->editor) {
        this->editor->setMode(MODE_NORMAL);
    }
}

void Window::openList() {
    this->filesLookup->show();
}

void Window::closeList() {
    this->filesLookup->hide();
}

void Window::openGrep(const QString& string) {
    this->openGrep(string, "");
}

void Window::openGrep(const QString& string, const QString& target) {
    if (target.size() > 0) {
        this->grep->grep(string, this->baseDir, target);
    } else {
        this->grep->grep(string, this->baseDir);
    }
    this->grep->show();
}

void Window::closeGrep() {
    this->grep->hide();
}

void Window::setCommand(const QString& text) {
    this->command->setText(text);
}

void Window::setBaseDir(const QString& dir) {
    if (dir.startsWith("/")) {
        this->baseDir = dir;
        return;
    }
    if (!this->baseDir.endsWith("/")) {
        this->baseDir += "/";
    }

    this->baseDir += dir;

    QFileInfo info(this->baseDir);
    this->baseDir = info.absoluteFilePath();
}

void Window::resizeEvent(QResizeEvent* event) {
    if (this->editor != nullptr) {
        this->editor->onWindowResized(event);
    }
}
