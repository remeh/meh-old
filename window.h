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

	void openCommand();
	void closeCommand();

protected:

private slots:

private:
	Editor* editor;
	QGridLayout* layout;
	Command* command;
};
