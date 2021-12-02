#include <QDir>
#include <QPushButton>
#include <QWidget>

#include "qdebug.h"

#include "breadcrumb.h"
#include "editor.h"
#include "window.h"

Breadcrumb::Breadcrumb(Window* window) :
    QWidget(nullptr),
    window(window), layout(nullptr) {
    this->deleteLabels();
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    this->layout = layout;
    this->setLayout(layout);

    this->setFocusPolicy(Qt::NoFocus);
    this->setMaximumHeight(26);
    this->setContentsMargins(0, 0, 0, 0);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void Breadcrumb::setFullpath(const QString& fullpath) {
    this->fullpath = fullpath;
    this->recomputeLabels();
}

void Breadcrumb::deleteLabels() {
    for (QPushButton* label : this->labels) {
        delete label;
    }

    this->labels.clear();

    if (this->layout != nullptr) {
        QLayoutItem *item = nullptr;
        QWidget* widget = nullptr;
        while ((item = this->layout->takeAt(0))) {
            if ((widget = item->widget())) {
                widget->hide();
                delete widget;
            }
            delete item;
        }
    }
}

void Breadcrumb::onClicked() {
    QObject* sender = QObject::sender();
    QString canonicalPath = sender->property("PATH").toString();
    if (this->window != nullptr) {
        if (canonicalPath.size() == 0) {
            this->window->getFilesLookup()->showBuffers();
        } else {
            this->window->getFilesLookup()->showFiles(canonicalPath);
        }
    }
}

void Breadcrumb::recomputeLabels() {
    this->deleteLabels();

    QDir d(this->fullpath);

    int textSize = 0;
    int displayCount = 0;

    QList<QPushButton*> buttons;
    bool first = true;
    do {
        QPushButton* label = new QPushButton();
        QFileInfo fi(d.canonicalPath());
        if (fi.isDir()) {
            label->setText(fi.fileName() + "/");
        } else {
            label->setText(fi.fileName());
        }
        label->setFont(Editor::getFont());
        label->setFlat(true);
        if (first) {
            label->setStyleSheet("padding: 0px; margin: 0px; font-weight: bold;");
        } else {
            label->setStyleSheet("padding: 0px; margin: 0px; color: #bbbbbb;");
        }

        if (first) {
            label->setProperty("PATH", "");
        } else {
            label->setProperty("PATH", d.canonicalPath());
        }

        buttons.prepend(label);
        first = false;

        connect(label, &QPushButton::clicked, this, &Breadcrumb::onClicked);

        textSize += label->text().size();
        ++displayCount;
        // minimum 2 entries (if text > 70), maximum 4 entries
        if ((textSize > 70 && displayCount > 1) || (displayCount > 3)) {
            break;
        }
    } while(d.cdUp());

    for (int i = 0; i < buttons.size(); ++i) {
        this->layout->addWidget(buttons.at(i));
    }
}

