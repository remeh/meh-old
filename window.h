#pragma once

#include <QGridLayout>
#include <QLineEdit>
#include <QWidget>

#include "editor.h"

class Command;
class Editor;

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QWidget* parent = nullptr);

	// openCommand opens the command line and focus on it.
	void openCommand();
	// closeCommand hides the command line and set the editor in normal mode
	// if it has been instanciated.
	void closeCommand();

	// setCommand sets the text in the command to the given value.
	void setCommand(const QString& text);

	Editor* getEditor() { return this->editor; }

protected:

private slots:

private:
	Editor* editor;
	QGridLayout* layout;
	Command* command;
};
