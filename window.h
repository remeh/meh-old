#pragma once

#include <QApplication>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QLocalServer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QString>
#include <QTabWidget>
#include <QWidget>

#include "editor.h"
#include "lsp.h"
#include "lsp_manager.h"

class Command;
class Completer;
class CompleterEntry;
class Editor;
class Exec;
class Git;
class Grep;
class ReferencesWidget;
class ReplaceWidget;
class StatusBar;

class Checkpoint {
    public:
        Checkpoint(const QString& id, int position) : id(id), position(position) {}
        QString id;
        int position;
};

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QApplication* app, QString instanceSocket, QWidget* parent = nullptr);
    ~Window();

    // openCommand opens the command line and focus on it.
    void openCommand();
    // closeCommand hides the command line and set the editor in normal mode
    // if it has been instanciated.
    void closeCommand();

    void openCompleter(const QString& base, QList<CompleterEntry> entries);
    void closeCompleter();

    // setCommand sets the text in the command to the given value.
    void setCommand(const QString& text);

    void openListFiles();
    void openListBuffers();

    void openReplace();
    void closeReplace();

    // closeList removes all entries in the list and hides it.
    // TODO(remy): rename me
    void closeList();

    void openGrep(const QString& string, const QString& target);
    void openGrep(const QString& string);
    void closeGrep();
    void focusGrep();

    // getStatusBar returns the StatusBar instance.
    StatusBar* getStatusBar() const { return this->statusBar; }

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


    // getRefWidget returns the ReferencesWidget instance.
    ReferencesWidget* getRefWidget() { return this->refWidget; }

    // getGit returns the Git instance.
    Git* getGit() { return this->git; }

    // getExec returns the Exec instance.
    Exec* getExec() { return this->exec; }

    // areYouSure shows a "Are you sure?" message box
    bool areYouSure();
    bool areYouSure(const QString& message);

    // buffers manipulation
    // --------------------

    // newEditor creates a new editor (which will create and manage a buffer)
    // and appends it to the list of managed editors.
    // The editor instance returned is managed by the Window.
    Editor* newEditor(QString name, QByteArray content);

    Editor* newEditor(QString name, QString filename);

    // getEditor returns the current Editor instance.
    Editor* getEditor();
    Editor* getEditor(const QString& id);
    Editor* getEditor(int tabIndex);
    QList<Editor*> getEditors();

    // getPreviousEditorId returns the id of the previous editor used, or an
    // empty string if none.
    const QString& getPreviousEditorId() { return this->previousEditorId; }

    // XXX(remy): comment me
    // If this id is not found, it tries to open it as a file.
    Editor* setCurrentEditor(QString id);

    // closeCurrentEditor closes the current editor (freeing its memory).
    // Note that this method is not saving the data.
    void closeCurrentEditor();

    // closeEditor closes the given editor (freeing its memory).
    // Note that this method is not saving the data.
    void closeEditor(const QString& id);

    // getEditorTabIndex returns the tab index of the given editor.
    int getEditorTabIndex(const QString& id);
    int getEditorTabIndex(Editor* editor);

    int getTabsCount() { return this->tabs->count(); }

    // hasBuffer returns true if a buffer has already been loaded in one
    // of the managed editors.
    bool hasBuffer(const QString& id);

    // save saves the buffer in the current editor.
    void save();

    // saveAll saves all the loaded buffers.
    void saveAll();

    // modifiedBuffers returns a list of the loaded and modified buffers that
    // would need to be stored on disk.
    QStringList modifiedBuffers();

    // saveCheckpoint stores the current filename/cursor position information has a checkpoint.
    void saveCheckpoint();

    // lastCheckpoint pops the last checkpoint and goes into this file / position.
    void lastCheckpoint();

    // lsp
    // -----------

    LSPManager* getLSPManager() { return this->lspManager; }

    // showLSPDiagnosticsOfLine shows the diagnostics for the given buffer and
    // the given line (of the current buffer) in the statusbar.
    void showLSPDiagnosticsOfLine(const QString& buffId, int line);

    // lspInterpretMessages is called when data has been received from the LSP server
    // and that we need to interpret them. Note that it can interpret several
    // messages in on call (if the LSP server has sent several messages in one output)
    void lspInterpretMessages(const QByteArray& data);

    // lspInterpret is called by the LSP manager to interpret one JSON message.
    void lspInterpret(QJsonDocument json);

protected:
    void paintEvent(QPaintEvent* event);

private slots:
    void resizeEvent(QResizeEvent*);
    void closeEvent(QCloseEvent*);
    void onCloseTab(int);
    void onChangeTab(int);
    void onNewSocketCommand();

private:
    QApplication* app;
    const QString instanceSocket;

    // tabs is containing all editors instances
    QTabWidget* tabs;

    QGridLayout* layout;
    Command* command;
    FilesLookup* filesLookup;
    Grep* grep;
    Completer* completer;
    ReferencesWidget *refWidget;
    StatusBar* statusBar;
    Git* git;
    Exec* exec;
    ReplaceWidget* replace;

    // baseDir on which the FilesLookup should be opened.
    QString baseDir;

    // previousEditorId stores the previously used editor to go back to it if
    // using ctrl-O
    QString previousEditorId;

    // checkpoints stored.
    QList<Checkpoint> checkpoints;

    QLocalServer commandServer;

    LSPManager* lspManager;
};
