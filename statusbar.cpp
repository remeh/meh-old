#include <QSizePolicy>

#include "editor.h"
#include "statusbar.h"
#include "window.h"

StatusBar::StatusBar(Window* window) :
    QWidget(window),
    window(window),
    lspWorking(false) {
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

    this->breadcrumb = new Breadcrumb(window);

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
    this->glayout->addWidget(this->breadcrumb, 0, 1, Qt::AlignCenter);
    this->glayout->addWidget(this->lineNumber, 0, 2, Qt::AlignRight);
    this->setFont(Editor::getFont());
    this->message = new QPlainTextEdit();
    this->message->setReadOnly(true);
    this->message->setFocusPolicy(Qt::NoFocus);
    this->message->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->message->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->message->setStyleSheet("color: #e7e7e7; background-color: #262626;");
    this->message->setFont(Editor::getFont());
    this->message->setMaximumHeight(150);
    this->vlayout->setContentsMargins(5, 0, 5, 0);
    this->vlayout->addWidget(this->message);
    this->vlayout->addLayout(glayout);
    this->setLayout(vlayout);

    this->hideMessage();
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

void StatusBar::setEditor(Editor* editor) {
    Q_ASSERT(editor != nullptr);

    switch (editor->getBuffer()->getType()) {
        case BUFFER_TYPE_GIT_BLAME:
            this->breadcrumb->setFullpath("GIT BLAME - " + editor->getBuffer()->getName());
            this->breadcrumb->setModified(editor->getBuffer()->modified);
            break;
        default:
            this->breadcrumb->setFullpath(editor->getBuffer()->getFilename());
            this->breadcrumb->setModified(editor->getBuffer()->modified);
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
    this->breadcrumb->setModified(modified);
}

void StatusBar::setLineNumber(int lineNumber) {
    if (this->lineNumber == nullptr) { return; }
    this->lineNumber->setText(QString::number(lineNumber));
}

void StatusBar::setLspRunning(bool running) {
    if (this->lspWorking == running) { return; }
    this->lspWorking = running;
    this->setMode(this->mode->text());
}
