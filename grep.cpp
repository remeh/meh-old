#include <QByteArray>
#include <QGridLayout>

#include "grep.h"
#include "window.h"

#include "qdebug.h"

Grep::Grep(Window* window) :
    QWidget(window),
    window(window) {
    Q_ASSERT(window != nullptr);

    this->process = nullptr;
    this->setFont(Editor::getFont());
}

void Grep::show() {
    this->window->getRefWidget()->show();
    QWidget::show();
    this->window->getRefWidget()->setFocus();
}

void Grep::focus() {
    this->window->getRefWidget()->setDisabled(false);
    this->window->getRefWidget()->setFocus();
}

void Grep::hide() {
    if (this->process != nullptr) {
        disconnect(this->process, &QProcess::readyReadStandardOutput, this, &Grep::onResults);
        disconnect(this->process, &QProcess::errorOccurred, this, &Grep::onErrorOccurred);
        disconnect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Grep::onFinished);
        this->process->terminate();
        delete this->process;
        this->process = nullptr;
    }
    this->resultsCount = 0;
    this->window->getRefWidget()->hide();
    QWidget::hide();
}

void Grep::onResults() {
    for (QByteArray data = this->process->readLine(); data.size() > 0; data = this->process->readLine()) {
        buff.append(data);
        if (buff.endsWith("\n")) {
            this->readAndAppendResult(buff);
            this->resultsCount++;
            buff.clear();
        }
    }

    this->window->getRefWidget()->setLabelText(" " + QString::number(this->resultsCount) + " results");
}

void Grep::onErrorOccurred() {
    if (this->process) {
        delete this->process;
        this->process = nullptr;
    }
}

void Grep::onFinished() {
    if (this->process) {
        delete this->process;
        this->process = nullptr;
    }

    this->window->getRefWidget()->fitContent();
    this->window->getRefWidget()->sort(0, Qt::AscendingOrder);
}

void Grep::keyPressEvent(QKeyEvent* event) {
    this->window->getRefWidget()->onKeyPressEvent(event);
}

void Grep::readAndAppendResult(const QString& result) {
    // first two are the file and the line number
    QStringList parts = result.split(":").mid(0, 2);
    // the rest is the result
    QString line = result.section(":", 2);
    parts << line.trimmed();

    if (parts[0].startsWith("./")) {
        parts[0] = parts[0].remove(0, 2);
    }

    if (!parts[0].startsWith(this->window->getBaseDir())) {
        parts[0] = this->window->getBaseDir() + parts[0];
    }

    this->window->getRefWidget()->insert(parts[0], parts[1], parts[2]);
}

int Grep::grep(const QString& string, const QString& baseDir) {
    int rv = this->grep(string, baseDir, "");
    this->window->getRefWidget()->clear();
    return rv;
}

// TODO(remy): support case insensitive
int Grep::grep(const QString& string, const QString& baseDir, const QString& target) {
    if (this->process != nullptr) {
        this->process->terminate();
        delete this->process;
        this->process = nullptr;
    }

    if (string.size() == 0 || string.trimmed().size() == 0) {
        this->window->getStatusBar()->setMessage("can't start a search with an empty string.");
        return -1;
    }

    // reinit
    this->resultsCount = 0;
    this->window->getRefWidget()->clear();

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo baseDirInfo(baseDir);
    this->process->setWorkingDirectory(baseDirInfo.canonicalFilePath());

    // target
    QString t = target;
    if (t.size() == 0) { t = "."; }
    QStringList list;
    list << "--with-filename" << "--line-number" << string << t;

    // run ripgrep
    this->process->start("rg", list);

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Grep::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Grep::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Grep::onFinished);

    return 0;
}
