#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

#define GIT_UNKNOWN 0
#define GIT_BLAME 1
#define GIT_SHOW

class Buffer;
class Window;

class Git : public QObject {
    Q_OBJECT

public:
    Git(Window* window);

    // blame is running git blame on the buffer file, and his opening the result
    // in a new buffer.
    void blame(const Buffer* buffer);

    // show is showing the buffer file statee in the given checksum.
    void show(const Buffer* buffer, const QString& checksum);

public slots:
    void onErrorOccurred();
    void onResults();
    void onFinished();

private:
    QByteArray data;
    QProcess* process;
    Window* window;

    // command contains the last command started
    int command;
};
