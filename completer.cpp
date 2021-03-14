#include <QTreeWidget>

#include "completer.h"
#include "window.h"

Completer::Completer(Window* window) :
    QTreeWidget(window),
    window(window) {

    QStringList columns;
    columns << "Completion" << "Infos";
    this->setHeaderLabels(columns);
}

void Completer::setItems(const QString& base, const QList<CompleterEntry> entries) {
    for (int i = 0; i < entries.size(); i++) {
        QStringList list; list << entries[i].completion << entries[i].infos;
        new QTreeWidgetItem(this, list);
    }
    this->base = base;
    this->fitContent();
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
            this->window->getEditor()->applyAutocomplete(this->base, this->currentItem()->text(0));
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