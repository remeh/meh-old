#pragma once

#include <QByteArray>
#include <QFile>
#include <QProcess>
#include <QTextEdit>
#include <QString>

// buffer is showing data, we don't know from where the data come from
#define BUFFER_TYPE_UNKNOWN	0
// buffer is showing content of file
#define BUFFER_TYPE_FILE		1
// buffer is showing a git blame
#define BUFFER_TYPE_GIT_BLAME	2
// buffer is showing a git show
#define BUFFER_TYPE_GIT_SHOW	3
// buffer is showing a git diff
#define BUFFER_TYPE_GIT_DIFF	4
// buffer is showing a command exec result
#define BUFFER_TYPE_COMMAND	5

class Editor;

class Buffer
{
public:
    Buffer();

    // Buffer creates a buffer targeting a given file.
    Buffer(QString filename);

    // Buffer creates a buffer showing the given data
    Buffer(QByteArray data);

    // read returns the content of the buffer. It reads the content from the file
    // on disk if has not been already done.
    QByteArray read();

    // save saves the file on disk.
    void save(Editor* editor);

    const QString& getFilename() const { return this->filename; }

    const QByteArray& getData() const { return this->data; }

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

    // getType returns the type of this buffer.
    int getType() { return this->bufferType; }

    // setType sets the type of this buffer.
    void setType(int type) { this->bufferType = type; }

    // setName sets the name of this buffer, which is used when there is no filename attached.
    void setName(const QString& name) { this->name = name; }

    // getName returns the name of this buffer, which is used when there is no filename attached.
    const QString& getName() { return this->name; }

    // getId returns an identifier for this buffer. This identifier will be generated using the
    // type of buffer.
    QString getId();

protected:

private:
    QString filename;
    QString name; // if the buffer doesn't has a filename attached, it may have a name

    bool readFromDisk;
    QByteArray data;

    int bufferType;
};