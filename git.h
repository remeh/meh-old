#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

#define GIT_UNKNOWN 0
#define GIT_BLAME   1
#define GIT_SHOW    2
#define GIT_DIFF    3

class Buffer;
class Window;

class Git : public QObject {
    Q_OBJECT

public:
    Git(Window* window);

    // blame is running git blame on the buffer file, and his opening the result
    // in a new buffer.
    void blame(const Buffer* buffer);

    // show is showing the given git commit with this checksum
    void show(const QString& baseDir, const QString& checksum);

    void diff(const QString& baseDir, bool staged);

    static bool isGitTempFile(const QString& filename);

public slots:
    void onErrorOccurred();
    void onResults();
    void onFinished();

private:
    QByteArray data;
    QProcess* process;
    Window* window;

    // bufferName is a temporary var used to set the buffer name after the command
    // has been executed.
    QString bufferName;

    // command contains the last command started
    int command;
};
