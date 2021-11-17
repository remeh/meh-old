#include <QTreeWidget>

#include "completer.h"
#include "window.h"

Completer::Completer(Window* window) :
    QTreeWidget(window),
    window(window) {

    QStringList columns;
    columns << "T" << "Completion" << "Infos";
    this->setHeaderLabels(columns);
    this->setColumnWidth(0, 5);
    this->setColumnWidth(1, 200);
    this->setColumnWidth(2, 500);
    this->setFont(Editor::getFont());
}

void Completer::setItems(const QString& base, const QList<CompleterEntry> entries) {
    for (int i = 0; i < entries.size(); i++) {
        QStringList list;

        if (entries[i].isFunc) {
            list << "f";
        } else {
            list << "";
        }

        list << entries[i].completion << entries[i].infos;

        new QTreeWidgetItem(this, list);
    }
    this->base = base;
    this->fitContent();
    this->setCurrentItem(this->topLevelItem(0));
}

void Completer::keyPressEvent(QKeyEvent* event) {
    #ifdef Q_OS_MAC
        bool ctrl = event->modifiers() & Qt::MetaModifier;
    #else
        bool ctrl = event->modifiers() & Qt::ControlModifier;
    #endif


    switch (event->key()) {
        case Qt::Key_Escape:
            this->window->closeCompleter();
            return;
        case Qt::Key_Return:
        case Qt::Key_Space:
            this->window->getEditor()->applyAutocomplete(this->currentItem()->text(0), this->base, this->currentItem()->text(1), this->currentItem()->text(2));
            this->window->closeCompleter();
            return;
        case Qt::Key_N:
            if (ctrl) {
                QTreeWidgetItem* currentItem = this->currentItem();
                if (currentItem == nullptr) {
                    currentItem = this->topLevelItem(0);
                    this->setCurrentItem(currentItem);
                    return;
                }
                currentItem = this->itemBelow(currentItem);
                if (currentItem != nullptr) {
                    this->setCurrentItem(currentItem);
                }

            }
            return;
        case Qt::Key_P:
            if (ctrl) {
                QTreeWidgetItem* currentItem = this->currentItem();
                if (currentItem == nullptr) {
                    currentItem = this->topLevelItem(0);
                    this->setCurrentItem(currentItem);
                    return;
                }
                currentItem = this->itemAbove(currentItem);
                if (currentItem != nullptr) {
                    this->setCurrentItem(currentItem);
                }
            }
            return;
    }
}

void Completer::fitContent() {
    this->resizeColumnToContents(0);
}

void Completer::show() {
    QRect cursorRect = this->window->getEditor()->cursorRect();
    int winWidth = this->window->size().width();
    int winHeight = this->window->size().height();
    QWidget::show();
    QWidget::raise();
    this->resize(705, 200);
    this->move(cursorRect.x() + 50, cursorRect.y() + 30);
}