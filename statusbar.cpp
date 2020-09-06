#include "statusbar.h"
#include "window.h"

StatusBar::StatusBar(Window* window) :
    QWidget(window),
    window(window) {
    Q_ASSERT(window != nullptr);
    this->layout = new QGridLayout();
    this->mode = new QLabel("Normal");
    this->mode->setFont(Editor::getFont());
    this->filename = new QLabel("");
    this->lineNumber = new QLabel("0");
    this->lineNumber->setFont(Editor::getFont());
    this->layout->setContentsMargins(10, 0, 10, 10);
    this->layout->addWidget(this->mode);
    this->layout->addWidget(this->filename, 0, 1, Qt::AlignCenter);
    this->layout->addWidget(this->lineNumber, 0, 2, Qt::AlignRight);
    this->setFont(Editor::getFont());
    this->setLayout(layout);
}

void StatusBar::setMode(const QString& mode) {
    if (this->mode == nullptr) { return; }
    this->mode->setText(mode);
}

void StatusBar::setFilename(const QString& filename) {
    if (this->filename == nullptr) { return; }
    this->filename->setText(filename);
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
