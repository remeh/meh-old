#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

#define GIT_UNKNOWN		0
#define GIT_BLAME	 		1
#define GIT_SHOW 			2
#define GIT_DIFF			3
#define GIT_DIFF_STAT 	4

class Buffer;
class Editor;

class Git : public QObject {
    Q_OBJECT

public:
    Git(Editor* editor);

    // blame is running git blame on the buffer file, and his opening the result
    // in a new buffer.
    void blame();

    // show is showing the given git commit with this checksum
    void show(const QString& baseDir, const QString& checksum);

    void diff(bool staged, bool stat = false);

    static bool isGitTempFile(const QString& filename);
    static bool isGitFile(const QString& filename);

public slots:
    void onErrorOccurred();
    void onResults();
    void onFinished();

private:
    QByteArray data;
    QProcess* process;
    Editor* editor;

    // bufferName is a temporary var used to set the buffer name after the command
    // has been executed.
    QString bufferName;

    // command contains the last command started
    int command;

    void processDiff(const QString& diff);
};
