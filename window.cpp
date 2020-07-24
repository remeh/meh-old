#include <QFileInfo>
#include <QString>

#include "command.h"
#include "completer.h"
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
    this->completer = new Completer(this);
    this->completer->setMaximumHeight(100);
    this->completer->hide();

    // editor
    // ----------------------

    this->editor = new Editor(this);

    // set base dir
    // ----------------------

    QFileInfo dir(".");
    this->setBaseDir(dir.absoluteFilePath());

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
    this->layout->addWidget(this->completer);
    this->setLayout(layout);
}

void Window::closeEvent(QCloseEvent* event) {
    this->command->execute(":q");
    event->ignore();
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

void Window::openListFiles() {
    this->filesLookup->showFiles();
}

void Window::openListBuffers() {
    this->filesLookup->showBuffers();
}

void Window::closeList() {
    this->filesLookup->hide();
}

void Window::openCompleter(const QString& base, const QStringList& list) {
    this->completer->clear();
    this->completer->setItems(base, list);
    QPoint p; this->mapToParent(p);
    this->completer->setGeometry(p.x(), p.y(), 200, 60);
    this->completer->show();
    this->completer->setFocus();
}

void Window::closeCompleter() {
    this->completer->hide();
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
    QFileInfo info(dir);
    this->baseDir = info.absoluteFilePath();
    if (!this->baseDir.endsWith("/")) {
        this->baseDir += "/";
    }
}

void Window::resizeEvent(QResizeEvent* event) {
    if (this->editor != nullptr) {
        this->editor->onWindowResized(event);
    }
}
