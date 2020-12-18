#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include "replace.h"
#include "window.h"

#include "qdebug.h"

ReplaceWidget::ReplaceWidget(Window* window) : QWidget(window), window(window) {
    this->layout = new QVBoxLayout();

    this->searchFor = new QLabel("Search for");
    this->searchForEdit = new QLineEdit();
    this->replaceWith = new QLabel("Replace with");
    this->replaceWithEdit = new QLineEdit();

    this->layout->addWidget(this->searchFor);
    this->layout->addWidget(this->searchForEdit);
    this->layout->addWidget(this->replaceWith);
    this->layout->addWidget(this->replaceWithEdit);

    this->setLayout(layout);
}

void ReplaceWidget::show() {
    this->clear();
    this->searchForEdit->setFocus();
    QWidget::show();
}

void ReplaceWidget::clear() {
    this->replaceWithEdit->setText("");
    this->searchForEdit->setText("");
}

void ReplaceWidget::replace() {
    // TODO(remy): implement me
}

void ReplaceWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        this->window->closeReplace();
        return;
    }

    if (event->key() == Qt::Key_Return) {
        if (this->searchForEdit->text() == "") {
            this->searchForEdit->setFocus();
            return;
        }

        this->replace();
        return;
    }
}