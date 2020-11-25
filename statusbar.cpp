#include <QSizePolicy>

#include "buffer.h"
#include "statusbar.h"
#include "window.h"

StatusBar::StatusBar(Window* window) :
    QWidget(window),
    window(window) {
    Q_ASSERT(window != nullptr);
    this->vlayout = new QVBoxLayout();
    this->glayout = new QGridLayout();
    this->mode = new QLabel("Normal");
    this->mode->setFont(Editor::getFont());
    #ifdef Q_OS_MAC
    this->mode->setMaximumHeight(18);
    #else
    this->mode->setMaximumHeight(15);
    #endif
    this->mode->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->filename = new QPushButton("");
    this->filename->setFlat(true);
    this->filename->setFocusPolicy(Qt::NoFocus);
    #ifdef Q_OS_MAC
    this->filename->setMaximumHeight(26);
    #else
    this->filename->setMaximumHeight(17);
    #endif
    this->filename->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->lineNumber = new QLabel("0");
    this->lineNumber->setFont(Editor::getFont());
    this->lineNumber->setFont(Editor::getFont());
    #ifdef Q_OS_MAC
    this->lineNumber->setMaximumHeight(18);
    #else
    this->lineNumber->setMaximumHeight(15);
    #endif
    this->lineNumber->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->glayout->setContentsMargins(10, 0, 10, 5);
    this->glayout->addWidget(this->mode);
    this->glayout->addWidget(this->filename, 0, 1, Qt::AlignCenter);
    this->glayout->addWidget(this->lineNumber, 0, 2, Qt::AlignRight);
    this->setFont(Editor::getFont());
    this->message = new QPlainTextEdit();
    this->message->setReadOnly(true);
    this->message->setFocusPolicy(Qt::NoFocus);
    this->message->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->message->setMaximumHeight(150);
    this->vlayout->setContentsMargins(5, 0, 5, 0);
    this->vlayout->addWidget(this->message);
    this->vlayout->addLayout(glayout);
    this->setLayout(vlayout);

    this->hideMessage();

    connect(this->filename, &QPushButton::clicked, this, &StatusBar::onFilenameClicked);
}

void StatusBar::onFilenameClicked() {
    if (this->window != nullptr) {
        this->window->openListBuffers();
    }
}

void StatusBar::setMode(const QString& newMode) {
    if (this->mode == nullptr) { return; }
    if (this->lspWorking) {
        this->mode->setText(newMode + " - LSP working");
    } else {
        QString tmp = newMode;
        this->mode->setText(tmp.replace(" - LSP working", ""));
    }
}

void StatusBar::setBuffer(Buffer* buffer) {
    if (this->filename == nullptr || buffer == nullptr) {
        return;
    }

    switch (buffer->getType()) {
        case BUFFER_TYPE_GIT_BLAME:
            this->filename->setText("GIT BLAME - " + buffer->getName());
            break;
        default:
            this->filename->setText(buffer->getFilename());
    }
}

void StatusBar::setMessage(const QString& message) {
    if (this->message == nullptr) { return; }
    QString t = this->message->toPlainText();
    if (t.size() > 0) {
        this->message->setPlainText(t + "\n" + message);
    } else {
        this->message->setPlainText(message);
    }
    this->showMessage();
}

void StatusBar::setModified(bool modified) {
    if (this->filename->text().endsWith("*")) {
        if (!modified) {
            int size = this->filename->text().size();
            this->filename->setText(this->filename->text().remove(size-1, size));
        }
    } else {
        if (modified) {
            this->filename->setText(this->filename->text() + "*");
        }
    }
}

void StatusBar::setLineNumber(int lineNumber) {
    if (this->lineNumber == nullptr) { return; }
    this->lineNumber->setText(QString::number(lineNumber));
}

void StatusBar::setLspRunning(bool running) {
    this->lspWorking = running;
    this->setMode(this->mode->text());
}