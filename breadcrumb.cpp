#include <QDir>
#include <QPushButton>

#include "breadcrumb.h"
#include "editor.h"

Breadcrumb::Breadcrumb(Window* window) :
    window(window) {
    this->layout = new QHBoxLayout();
    this->setLayout(layout);

    this->setFocusPolicy(Qt::NoFocus);
    #ifdef Q_OS_MAC
    this->setMaximumHeight(26);
    #else
    this->setMaximumHeight(17);
    #endif
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
}

void Breadcrumb::recomputeLabels() {
    qDebug() << "recomputeLabels entering";
    this->deleteLabels();

    QDir d(this->fullpath);

    QList<QPushButton*> reversed;
    do {
        QPushButton* label = new QPushButton(this);
        QFileInfo fi(d.canonicalPath());
        label->setText(fi.fileName());
        label->setFlat(true);
        reversed.append(label);
    } while(d.cdUp());

    for (int i = reversed.size() - 1; i >= 0; i--) {
        qDebug() << reversed.at(i);
        this->layout->addWidget(reversed.at(i));
    }

    qDebug() << "recomputeLabels done";
}

void Breadcrumb::mousePressEvent(QMouseEvent* event) {
}