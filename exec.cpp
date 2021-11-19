#include <QList>
#include <QString>

#include "buffer.h"
#include "exec.h"
#include "git.h"
#include "window.h"

Exec::Exec(Window* window) : window(window), command("") {
    this->process = nullptr;
}

void Exec::onResults() {
    if (!this->process) {
        return;
    }

    for (QByteArray readData = this->process->readLine(); readData.size() > 0; readData = this->process->readLine()) {
        this->data.append(readData);
    }
}

void Exec::onErrorOccurred(QProcess::ProcessError error) {
    this->process->deleteLater();

    this->window->getStatusBar()->setMessage("An error occurred while running: '" + this->command + "':\n" + QVariant::fromValue(error).toString());
    this->window->getStatusBar()->showMessage();
}

void Exec::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (this->process) {
        delete this->process;
        this->process = nullptr;
    }

    if (this->command.startsWith("fd ")) {
        QList<QByteArray> split = this->data.split('\n');
        QList<QString> files;
        for (QByteArray arr : split) {
            if (!arr.isEmpty()) {
                files.append(QString(arr));
            }
        }
        this->window->getFilesLookup()->showList(files);
        this->window->getFilesLookup()->setLabel(":!" + this->command);
    } else {
        if (this->data.size() == 0) {
            this->window->getStatusBar()->setMessage(this->command + QString("\n\nCommand executed but no output."));
            this->window->getStatusBar()->showMessage();
            return;
        }

        Editor* editor = this->window->newEditor(this->command, this->data);
        // XXX(remy): set type
        editor->getBuffer()->setType(BUFFER_TYPE_COMMAND);
        editor->goToLine(0);
    }

    // we've finished, clean-up
    this->data.clear();
    this->command = "";
}


void Exec::start(const QString& baseDir, QStringList args) {
    this->command = args.join(" ");

    this->process = new QProcess(this);

    this->process->setWorkingDirectory(baseDir);
    this->process->setProcessChannelMode(QProcess::MergedChannels); // read both stdout and stderr

    if (args.size() < 1) {
        this->window->getStatusBar()->setMessage(QString("can't execute command:") + QString(args.join(" ")));
        return;
    }

    const QString& command = args[0];
    args.removeFirst();

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Exec::onResults);
    connect(this->process, &QProcess::readyReadStandardError, this, &Exec::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Exec::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Exec::onFinished);

    this->process->start(command, args);
}
