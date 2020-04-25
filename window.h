#pragma once

#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QResizeEvent>
#include <QWidget>

#include "editor.h"

class Command;
class Editor;
class Grep;

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

	// showList shows the list below the command field at the bottom of the window.
	// TODO(remy): rename me
	void openList();

	// closeList removes all entries in the list and hides it.
	// TODO(remy): rename me
	void closeList();

	// TODO(remy): comment me
	void openGrep(const QString& string);
	// TODO(remy): comment me
	void closeGrep();

	// setBaseDir sets the base dir on which the FilesLookup
	// should be opened.
	void setBaseDir(const QString& dir);
	QString getBaseDir() { return this->baseDir; }

	Editor* getEditor() { return this->editor; }

protected:

private slots:
	void resizeEvent(QResizeEvent*);

private:
	Editor* editor;
	QGridLayout* layout;
	Command* command;
	FilesLookup* filesLookup;
	Grep* grep;

	// baseDir on which the FilesLookup should be opened.
	QString baseDir;
};
