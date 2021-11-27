#include <QRegularExpression>

#include "buffer.h"
#include "editor.h"
#include "git.h"
#include "line_number_area.h"
#include "window.h"

Git::Git(Editor* editor) : editor(editor), command(GIT_UNKNOWN), bufferName("") {
    this->process = nullptr;
}

void Git::onResults() {
    for (QByteArray readData = this->process->readLine(); readData.size() > 0; readData = this->process->readLine()) {
        this->data.append(readData);
    }
}

void Git::onErrorOccurred() {
    if (this->process) {
        this->editor->getWindow()->getStatusBar()->setMessage("An error occurred while running git blame.");
        delete this->process;
        this->process = nullptr;
        this->editor = nullptr;
    }
}

void Git::onFinished() {
    if (this->process) {
        delete this->process;
        this->process = nullptr;
    }

    int lineNumber = this->editor->currentLineNumber();

    switch (this->command) {
        case GIT_BLAME:
            {
                QString str = this->data;
                str = str.replace(QRegularExpression("\\)\\s*\n"), ")\n");
                Editor* newEditor = this->editor->getWindow()->newEditor(this->bufferName, str.toUtf8());
                newEditor->getBuffer()->setType(BUFFER_TYPE_GIT_BLAME);
                newEditor->goToLine(lineNumber);
            }
            break;
        case GIT_SHOW:
            {
                QString str = this->data;
                str = str.replace(QRegularExpression("\n\\s*\n"), "\n\n");
                Editor* newEditor = this->editor->getWindow()->newEditor(this->bufferName, str.toUtf8());
                newEditor->getBuffer()->setType(BUFFER_TYPE_GIT_SHOW);
                newEditor->goToLine(0);
            }
            break;
        case GIT_DIFF:
            {
                QString str = this->data;
                str = str.replace(QRegularExpression("\n\\s*\n"), "\n\n");
                Editor* newEditor = this->editor->getWindow()->newEditor(this->bufferName, str.toUtf8());
                newEditor->getBuffer()->setType(BUFFER_TYPE_GIT_DIFF);
                newEditor->goToLine(0);
            }
            break;
        case GIT_DIFF_STAT:
            {
                QString diff = this->data;
                this->processDiff(diff);
                this->editor->update();
            }
            break;
    }

    // we've finished, clean-up
    this->data.clear();
    this->command = GIT_UNKNOWN;
}

void Git::processDiff(const QString& diff) {
    // TODO(remy): limit the processing to a maximum size here?
    this->editor->lineNumberArea->clearGitFlags();

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
            this->editor->lineNumberArea->setGitFlag(newCounter, GIT_FLAG_REMOVED);
            continue;
        }

        if (line.startsWith("+")) {
            this->editor->lineNumberArea->setGitFlag(newCounter, GIT_FLAG_ADDED);
        }

        newCounter++;
    }
}

void Git::blame() {
    Q_ASSERT(this->editor != nullptr);
    Q_ASSERT(this->editor->getBuffer() != nullptr);

    // TODO(remy): runGit(QStringList args, int command) command

    const QString& filename = this->editor->getBuffer()->getFilename();
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
    Q_ASSERT(this->editor != nullptr);
    Q_ASSERT(this->editor->getBuffer() != nullptr);

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

void Git::diff(bool staged, bool stat) {
    Q_ASSERT(this->editor != nullptr);
    Q_ASSERT(this->editor->getBuffer() != nullptr);

    const Buffer* buffer = this->editor->getBuffer();

    this->data.clear(); // clear the data read from the process before starting the show

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