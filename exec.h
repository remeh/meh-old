#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class Buffer;
class Window;

class Exec : public QObject {
    Q_OBJECT

public:
    Exec(Window* window);

    // start runs the command and display the result in a buffer
    void start(const QString& baseDir, QStringList args);

public slots:
    void onErrorOccurred(QProcess::ProcessError error);
    void onResults();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QByteArray data;
    QProcess* process;
    Window* window;

    QString command;
};
