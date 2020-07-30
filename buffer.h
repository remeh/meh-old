#pragma once

#include <QByteArray>
#include <QFile>
#include <QProcess>
#include <QTextEdit>
#include <QString>

class Editor;

class Buffer
{
public:
    Buffer();
    // Buffer creates a buffer targeting a given file.
    Buffer(QString filename);

    // read returns the content of the buffer. It reads the content from the file
    // on disk if has not been already done.
    QByteArray read();

    // save saves the file on disk.
    void save(Editor* editor);

    const QString& getFilename() { return this->filename; }

    const QByteArray& getData() { return this->data; }

    // refreshData refreshes the data of the current buffer with what's available
    // in the editor data.
    void refreshData(Editor* editor);

    // onLeave is called when the Window is leaving this Buffer (to show another one).
    void onLeave(Editor* editor);

    // onClose is called when the current Buffer is being closed.
    // onClose does NOT call onLeave.
    void onClose(Editor* editor);

    // onEnter is called when the window is starting to display this buffer.
    void onEnter(Editor* editor);

    // postProcess applies post processing to the current file.
    // Returns true if the file has changed since saving and should be reloaded.
    bool postProcess(Editor* editor);

    // modified is true if something has changed in the buffer which has not be
    // stored on disk.
    bool modified;

    // isGitTempFile returns true if the currently opened file is a git tmp file.
    bool isGitTempFile();

protected:

private:
    QString filename;

    bool readFromDisk;
    QByteArray data;
};