#pragma once

#include <QObject>
#include <QProcess>

class Buffer;
class Window;

class Git : public QObject {
    Q_OBJECT

public:
    Git(Window* window);

    // blame is running git blame on the opened buffer, and his opening the result
    // in a new buffer.
    void blame(const Buffer* buffer);

public slots:
    void onErrorOccurred();
    void onResults();
    void onFinished();

private:
    QByteArray data;
    QProcess* process;
    Window* window;
};
