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
    this->filename = new QPushButton("");
    this->filename->setFlat(true);
	this->filename->setFocusPolicy(Qt::NoFocus);
    this->lineNumber = new QLabel("0");
    this->lineNumber->setFont(Editor::getFont());
    this->message = new QLabel("");
    this->message->setWordWrap(true);
    this->hideMessage();
    this->lineNumber->setFont(Editor::getFont());
    this->glayout->setContentsMargins(10, 0, 10, 5);
    this->glayout->addWidget(this->mode);
    this->glayout->addWidget(this->filename, 0, 1, Qt::AlignCenter);
    this->glayout->addWidget(this->lineNumber, 0, 2, Qt::AlignRight);
    this->setFont(Editor::getFont());
    this->vlayout->setContentsMargins(10, 0, 10, 0);
    this->vlayout->addWidget(this->message);
    this->vlayout->addLayout(glayout);
    this->setLayout(vlayout);

    connect(this->filename, &QPushButton::clicked, this, &StatusBar::onFilenameClicked);
}

void StatusBar::onFilenameClicked() {
    if (this->window != nullptr) {
        this->window->openListBuffers();
    }
}

void StatusBar::setMode(const QString& mode) {
    if (this->mode == nullptr) { return; }
    this->mode->setText(mode);
}

void StatusBar::setFilename(const QString& filename) {
    if (this->filename == nullptr) { return; }
    this->filename->setText(filename);
}

void StatusBar::setMessage(const QString& message) {
    if (this->message == nullptr) { return; }
    this->message->setText(message);
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
