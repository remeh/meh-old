#pragma once

#include <QCloseEvent>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QResizeEvent>
#include <QString>
#include <QWidget>

#include "editor.h"

class Command;
class Completer;
class Editor;
class Grep;
class ReferencesWidget;
class StatusBar;

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

    // TODO(remy): comment me
    void openCompleter(const QString& base, const QStringList& list);
    // TODO(remy): comment me
    void closeCompleter();

    // setCommand sets the text in the command to the given value.
    void setCommand(const QString& text);

    // TODO(remy): rename and comment me
    void openListFiles();
    // TODO(remy): rename and comment me
    void openListBuffers();

    // closeList removes all entries in the list and hides it.
    // TODO(remy): rename me
    void closeList();

    // TODO(remy): comment me
    void openGrep(const QString& string, const QString& target);
    void openGrep(const QString& string);
    void closeGrep();
    void focusGrep();

    // getStatusBar returns the StatusBar instance.
    StatusBar* getStatusBar() { return this->statusBar; }

    // setBaseDir sets the base dir on which the FilesLookup
    // should be opened.
    void setBaseDir(const QString& dir);

    // getBaseDir returns the base dir on which the editor has been opened.
    // Always end with a /
    QString getBaseDir() {
        if (!this->baseDir.endsWith("/")) {
            return this->baseDir + "/";
        }
        return this->baseDir;
    }

    // getEditor returns the Editor instance.
    Editor* getEditor() { return this->editor; }

    // getRefWidget returns the ReferencesWidget instance.
    ReferencesWidget* getRefWidget() { return this->refWidget; }

protected:

private slots:
    void resizeEvent(QResizeEvent*);
    void closeEvent(QCloseEvent*);

private:
    Editor* editor;
    QGridLayout* layout;
    Command* command;
    FilesLookup* filesLookup;
    Grep* grep;
    Completer* completer;
    ReferencesWidget *refWidget;
    StatusBar* statusBar;

    // baseDir on which the FilesLookup should be opened.
    QString baseDir;
};
