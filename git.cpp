#include "buffer.h"
#include "git.h"
#include "window.h"

Git::Git(Window* window) : window(window) {
    this->process = nullptr;
}


void Git::onResults() {
    for (QByteArray readData = this->process->readLine(); readData.size() > 0; readData = this->process->readLine()) {
        this->data.append(readData);
    }
}

void Git::onErrorOccurred() {
    if (this->process) {
        this->window->getStatusBar()->setMessage("An error occurred while running git blame.");
        delete this->process;
        this->process = nullptr;
    }
}

void Git::onFinished() {
    if (this->process) {
        delete this->process;
        this->process = nullptr;
    }

    int lineNumber = this->window->getEditor()->currentLineNumber();

    Buffer* buffer = new Buffer(this->data);
    this->window->getEditor()->setCurrentBuffer(buffer);
    this->window->getEditor()->goToLine(lineNumber);
}


void Git::blame(const Buffer* buffer) {
    if (buffer == nullptr) {
        this->window->getStatusBar()->setMessage("Git blame called on a null buffer.");
        return;
    }

    const QString& filename = buffer->getFilename();
    this->data.clear(); // clear the data before starting the blame

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo fi(filename);

    this->process->setWorkingDirectory(fi.absolutePath());

    // run git blame <filename>
    QStringList args; args << "blame" << fi.fileName();
    this->process->start("git", args);

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Git::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Git::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Git::onFinished);
}