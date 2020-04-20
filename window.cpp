#include "command.h"
#include "editor.h"
#include "window.h"

Window::Window(QWidget* parent) :
	QWidget(parent),
	editor(nullptr) {
	// widgets
	// ----------------------
	this->command = new Command(this);
	this->command->hide();
	this->list = new QListWidget(this);
	new QListWidgetItem("main.cpp", this->list);
	new QListWidgetItem("editor.cpp", this->list);
	new QListWidgetItem("editor.h", this->list);
	new QListWidgetItem("syntax.cpp", this->list);
	new QListWidgetItem("syntax.h", this->list);
	this->list->hide();

	// editor
	// ----------------------

	this->editor = new Editor(this);

	Buffer* buffer = new Buffer("editor.cpp");
	editor->setCurrentBuffer(buffer);

	// layout
	// ----------------------

	this->layout = new QGridLayout();
	this->layout->setContentsMargins(0, 0, 0, 0);
	this->layout->addWidget(this->editor);
	this->layout->addWidget(this->command);
	this->layout->addWidget(this->list);
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
	this->list->show();
}

void Window::closeList() {
	this->list->hide();
}

void Window::setCommand(const QString& text) {
	this->command->setText(text);
}
