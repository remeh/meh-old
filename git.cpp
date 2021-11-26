#include <QRegularExpression>

#include "buffer.h"
#include "editor.h"
#include "git.h"
#include "line_number_area.h"
#include "window.h"

Git::Git(Window* window) : window(window), command(GIT_UNKNOWN), bufferName("") {
    this->process = nullptr;
    this->statEditor = nullptr;
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
        this->statEditor = nullptr;
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
                QString str = this->data;
                str = str.replace(QRegularExpression("\\)\\s*\n"), ")\n");
                Editor* editor = this->window->newEditor(this->bufferName, str.toUtf8());
                editor->getBuffer()->setType(BUFFER_TYPE_GIT_BLAME);
                editor->goToLine(lineNumber);
            }
            break;
        case GIT_SHOW:
            {
                QString str = this->data;
                str = str.replace(QRegularExpression("\n\\s*\n"), "\n\n");
                Editor* editor = this->window->newEditor(this->bufferName, str.toUtf8());
                editor->getBuffer()->setType(BUFFER_TYPE_GIT_SHOW);
                editor->goToLine(0);
            }
            break;
        case GIT_DIFF:
            {
                QString str = this->data;
                str = str.replace(QRegularExpression("\n\\s*\n"), "\n\n");
                Editor* editor = this->window->newEditor(this->bufferName, str.toUtf8());
                editor->getBuffer()->setType(BUFFER_TYPE_GIT_DIFF);
                editor->goToLine(0);
            }
            break;
        case GIT_DIFF_STAT:
            {
                QString diff = this->data;
                this->processDiff(diff);
                this->statEditor->update();
            }
            break;
    }

    // we've finished, clean-up
    this->data.clear();
    this->command = GIT_UNKNOWN;
    this->statEditor = nullptr;
}

void Git::processDiff(const QString& diff) {
    if (this->statEditor == nullptr) {
        return;
    }

    // TODO(remy): limit the processing to a maximum size here?
    this->statEditor->lineNumberArea->clearGitFlags();

    QRegularExpression rx(QStringLiteral("@@ -(\\d*),(\\d*) \\+(\\d*),(\\d*) @@"));
    QStringList lines = diff.split("\n");
    int oldCounter = 0;
    int newCounter = 0;
    bool parsing = false;
    for (QString line : lines) {
        if (line.contains(rx)) {
            QRegularExpressionMatchIterator it = rx.globalMatch(line);
            if (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
//                int oldStartLine = match.captured(1).toInt() + 3;
//                int oldEndLine = oldStartLine + match.captured(2).toInt()-6;

//                int newStartLine = match.captured(3).toInt() + 3;
//                int newEndLine = newStartLine + match.captured(4).toInt()-6;
//
//                for (int i = oldStartLine; i < oldEndLine; i++) {
//                    this->statEditor->lineNumberArea->setGitFlag(i, GIT_FLAG_REMOVED);
//                }
//                for (int i = newStartLine; i < newEndLine; i++) {
//                    this->statEditor->lineNumberArea->setGitFlag(i, GIT_FLAG_ADDED);
//                }
                oldCounter = match.captured(1).toInt();
                newCounter = match.captured(3).toInt();
            }
            parsing = true;
            continue;
        }

        if (!parsing) {
            continue;
        }

        if (line.startsWith("-")) {
            this->statEditor->lineNumberArea->setGitFlag(newCounter, GIT_FLAG_REMOVED);
            continue;
        }

        if (line.startsWith("+")) {
            this->statEditor->lineNumberArea->setGitFlag(newCounter, GIT_FLAG_ADDED);
        }

        newCounter++;
    }
}

void Git::blame(const Buffer* buffer) {
    if (buffer == nullptr) {
        this->window->getStatusBar()->setMessage("Git blame called on a null buffer.");
        return;
    }

    // TODO(remy): runGit(QStringList args, int command) command

    const QString& filename = buffer->getFilename();
    this->data.clear(); // clear the data read from the process before starting the blame

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo fi(filename);

    this->process->setWorkingDirectory(fi.canonicalPath());

    // run git blame <filename>
    QStringList args; args << "blame" << fi.fileName();
    this->process->start("git", args);

    this->command = GIT_BLAME;
    this->bufferName = fi.fileName();

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Git::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Git::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Git::onFinished);
}

void Git::show(const QString& workDir, const QString& checksum) {
    this->data.clear(); // clear the data read from the process before starting the show

    // TODO(remy): runGit(QStringList args, int command) command

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo fi(workDir);
    this->process->setWorkingDirectory(fi.canonicalPath());

    // run git show <checksum>
    QStringList args; args << "show" << checksum;
    this->process->start("git", args);

    this->command = GIT_SHOW;
    this->bufferName = checksum;

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Git::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Git::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Git::onFinished);
}

void Git::diff(Editor* editor, bool staged, bool stat) {
    if (editor == nullptr || editor->getBuffer() == nullptr) {
        return;
    }

    const Buffer* buffer = editor->getBuffer();

    this->data.clear(); // clear the data read from the process before starting the show
    this->statEditor = nullptr;

    // TODO(remy): runGit(QStringList args, int command) command

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo fi(buffer->getFilename());
    if (fi.isDir()) {
        return;
    }
    this->process->setWorkingDirectory(fi.canonicalPath());

    QStringList args; args << "diff";
    if (staged) {
        args << "--staged";
    }
    args << buffer->getFilename();
    this->process->start("git", args);

    if (stat) {
        this->command = GIT_DIFF_STAT;
    } else {
        this->command = GIT_DIFF;
    }

    this->statEditor = editor;
    this->bufferName = QString("DIFF - ") + QFileInfo(buffer->getFilename()).fileName();

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Git::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Git::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Git::onFinished);
}

bool Git::isGitTempFile(const QString& filename) {
    return filename.endsWith(".git/COMMIT_EDITMSG") || filename.endsWith(".git/MERGE_MSG");
}

bool Git::isGitFile(const QString &filename) {
    if (Git::isGitTempFile(filename)) {
        return true;
    }

    if (filename.toLower().contains("git")) {
        return true;
    }

    return false;
}