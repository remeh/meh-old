#include "buffer.h"
#include "exec.h"
#include "git.h"
#include "window.h"

Exec::Exec(Window* window) : window(window), command("") {
    this->process = nullptr;
}

void Exec::onResults() {
    for (QByteArray readData = this->process->readLine(); readData.size() > 0; readData = this->process->readLine()) {
        this->data.append(readData);
    }
}

void Exec::onErrorOccurred() {
    if (this->process) {
        this->window->getStatusBar()->setMessage("An error occurred while running git blame.");
        delete this->process;
        this->process = nullptr;
    }
}

void Exec::onFinished() {
    if (this->process) {
        delete this->process;
        this->process = nullptr;
    }

    if (this->data.size() == 0) {
        this->window->getStatusBar()->setMessage(this->command + QString("\n\nCommand executed but no output."));
        this->window->getStatusBar()->showMessage();
        return;
    }

    Buffer* buffer = new Buffer(this->data);
    buffer->setType(BUFFER_TYPE_COMMAND);
    buffer->setName(this->command);
    this->window->getEditor()->setCurrentBuffer(buffer);
    this->window->getEditor()->goToLine(0);

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
    this->process->start(command, args);

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Exec::onResults);
    connect(this->process, &QProcess::readyReadStandardError, this, &Exec::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Exec::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Exec::onFinished);
}
