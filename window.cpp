#include <QFileInfo>
#include <QString>

#include "command.h"
#include "completer.h"
#include "editor.h"
#include "fileslookup.h"
#include "grep.h"
#include "statusbar.h"
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

    // status bar
    // ----------------------
    this->statusBar = new StatusBar(this);
    this->statusBar->show();

    // editor
    // ----------------------

    this->editor = new Editor(this);

    // set base dir
    // ----------------------

    QFileInfo dir(".");
    this->setBaseDir(dir.absoluteFilePath());

    // references widget displayed at the bottom of the editor
    // ----------------------

    this->refWidget = new ReferencesWidget(this);
    this->refWidget->hide();

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
    this->layout->addWidget(this->completer);
    this->layout->addWidget(this->refWidget);
    this->layout->addWidget(this->statusBar);
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

void Window::openCompleter(const QString& base, const QList<CompleterEntry> entries) {
    this->completer->clear();
    this->completer->setItems(base, entries);
    QPoint p; this->mapToParent(p);
    this->completer->setGeometry(p.x(), p.y(), 200, 60);
    this->completer->show();
    this->completer->setFocus();
    if (entries.size() > 0) {
        this->completer->setCurrentItem(this->completer->topLevelItem(0));
    }
    this->completer->scroll(0, 0);
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

void Window::focusGrep() {
    this->grep->focus();
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
