#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <QTextCursor>
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
    this->status = new QLabel("");

    this->layout->addWidget(this->searchFor);
    this->layout->addWidget(this->searchForEdit);
    this->layout->addWidget(this->replaceWith);
    this->layout->addWidget(this->replaceWithEdit);
    this->layout->addWidget(this->status);

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
    this->status->setText("");
}

void ReplaceWidget::replace() {
    QTextCursor cursor = this->window->getEditor()->textCursor();

    QString str;
    if (cursor.hasSelection()) {
        str = cursor.selectedText();
    } else {
        str = this->window->getEditor()->document()->toPlainText();
    }

    QRegExp rx(this->searchForEdit->text());

    int count = str.count(rx);
    str.replace(rx, this->replaceWithEdit->text());

    cursor.beginEditBlock();
    if (!cursor.hasSelection()) {
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    }
    cursor.removeSelectedText();
    cursor.insertText(str);
    cursor.endEditBlock();

    if (count > 0) {
        this->status->setText(QString::number(count) + " entries replaced.");
    } else {
        this->status->setText("Nothing has been replaced.");
    }
}

void ReplaceWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        this->window->closeReplace();
        return;
    }

    if (event->key() == Qt::Key_Return) {
        if (this->searchForEdit->text() == "") {
            this->status->setText("Enter a valid regexp to search for.");
            this->searchForEdit->setFocus();
            return;
        }

        this->replace();
        return;
    }
}