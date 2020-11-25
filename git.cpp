#include "buffer.h"
#include "git.h"
#include "window.h"

Git::Git(Window* window) : window(window), command(GIT_UNKNOWN) {
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

    switch (this->command) {
        case GIT_BLAME:
            {
                Buffer* buffer = new Buffer(this->data);
                buffer->setType(BUFFER_TYPE_GIT_BLAME);
                // FIXME(remy): this is wrong if the user has switched buffer before the git process finished.
                buffer->setName(this->window->getEditor()->getCurrentBuffer()->getFilename());
                this->window->getEditor()->setCurrentBuffer(buffer);
                this->window->getEditor()->goToLine(lineNumber);
            }
            break;
        case GIT_SHOW:
            {
                Buffer* buffer = new Buffer(this->data);
                buffer->setType(BUFFER_TYPE_GIT_SHOW);
                // FIXME(remy): this is wrong if the user has switched buffer before the git process finished.
                buffer->setName(this->window->getEditor()->getCurrentBuffer()->getFilename());
                this->window->getEditor()->setCurrentBuffer(buffer);
                this->window->getEditor()->goToLine(lineNumber);
            }
            break;
    }

    // we've finished, clean-up
    this->data.clear();
    this->command = GIT_UNKNOWN;
}


void Git::blame(const Buffer* buffer) {
    if (buffer == nullptr) {
        this->window->getStatusBar()->setMessage("Git blame called on a null buffer.");
        return;
    }

    const QString& filename = buffer->getFilename();
    this->data.clear(); // clear the data read from the process before starting the blame

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo fi(filename);

    this->process->setWorkingDirectory(fi.absolutePath());

    // run git blame <filename>
    QStringList args; args << "blame" << fi.fileName();
    this->process->start("git", args);

    this->command = GIT_BLAME;

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Git::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Git::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Git::onFinished);
}

void Git::show(const Buffer* buffer, const QString& checksum) {
    if (buffer == nullptr) {
        this->window->getStatusBar()->setMessage("Git show called on a null buffer.");
        return;
    }

    const QString& filename = buffer->getFilename();
    this->data.clear(); // clear the data read from the process before starting the show

    // TODO(remy): runGit(QStringList args, int command) command

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo fi(filename);

    this->process->setWorkingDirectory(fi.absolutePath());

    // run git show <filename>:<checksum>
    QStringList args; args << "show" << fi.fileName()+":"+checksum;
    this->process->start("git", args);

    this->command = GIT_SHOW;

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Git::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Git::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Git::onFinished);
}