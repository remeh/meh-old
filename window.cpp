#include "command.h"
#include "editor.h"
#include "fileslookup.h"
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

	// layout
	// ----------------------

	this->layout = new QGridLayout();
	this->layout->setContentsMargins(0, 0, 0, 0);
	this->layout->addWidget(this->editor);
	this->layout->addWidget(this->command);
	this->layout->addWidget(this->filesLookup);
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
	this->filesLookup->lookupDir("../");
	this->filesLookup->show();
}

void Window::closeList() {
	this->filesLookup->hide();
}

void Window::setCommand(const QString& text) {
	this->command->setText(text);
}
