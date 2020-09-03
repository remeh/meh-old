#include <QByteArray>
#include <QGridLayout>

#include "grep.h"
#include "window.h"

#include "qdebug.h"

Grep::Grep(Window* window) :
    QWidget(window),
    window(window) {
    Q_ASSERT(window != nullptr);

    this->refWidget = new ReferencesWidget(window, this);
    this->process = nullptr;

    this->layout = new QGridLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->addWidget(this->refWidget);
    this->setLayout(layout);
}

void Grep::show() {
    this->refWidget->show();
    QWidget::show();
    this->refWidget->setFocus();
}

void Grep::hide() {
    if (this->process != nullptr) {
        this->process->terminate();
        delete this->process;
        this->process = nullptr;
    }
    this->resultsCount = 0;
    this->refWidget->hide();
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

    this->refWidget->setLabelText(" " + QString::number(this->resultsCount) + " results");
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

    this->refWidget->fitContent();
}

void Grep::keyPressEvent(QKeyEvent* event) {
    this->refWidget->onKeyPressEvent(event);
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

    this->refWidget->insert(parts[0], parts[1], parts[2]);
}

void Grep::grep(const QString& string, const QString& baseDir) {
    this->grep(string, baseDir, "");
    this->refWidget->clear();
}

// TODO(remy): support case insensitive
void Grep::grep(const QString& string, const QString& baseDir, const QString& target) {
    if (this->process != nullptr) {
        this->process->terminate();
        delete this->process;
        this->process = nullptr;
    }

    // reinit
    this->resultsCount = 0;
    this->refWidget->clear();

    // create and init the process
    this->process = new QProcess(this);
    QFileInfo baseDirInfo(baseDir);
    this->process->setWorkingDirectory(baseDirInfo.absoluteFilePath());

    // target
    QString t = target;
    if (t.size() == 0) { t = "."; }
    QStringList list; list << "--with-filename" << "--line-number" << string << t;

    // run ripgrep
    this->process->start("rg", list);

    // connect the events
    connect(this->process, &QProcess::readyReadStandardOutput, this, &Grep::onResults);
    connect(this->process, &QProcess::errorOccurred, this, &Grep::onErrorOccurred);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Grep::onFinished);
}
