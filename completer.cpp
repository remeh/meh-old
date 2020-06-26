#include <QListWidget>

#include "completer.h"
#include "window.h"

Completer::Completer(Window* window) :
    QListWidget(window),
    window(window) {
}

void Completer::setItems(const QString& base, const QStringList& list) {
    for (int i = 0; i < list.size(); i++) {
        new QListWidgetItem(list.at(i), this);
    }
    this->base = base;
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
            this->window->getEditor()->applyAutocomplete(this->base, this->currentItem()->text());
            this->window->closeCompleter();
            return;
        case Qt::Key_Space:
            this->window->getEditor()->applyAutocomplete(this->base, this->currentItem()->text());
            // TODO(remy): should we insert a space here?
            this->window->closeCompleter();
            return;
        case Qt::Key_N:
            if (ctrl) {
                this->setCurrentRow(this->currentRow() + 1);
            }
            return;
        case Qt::Key_P:
            if (ctrl) {
                this->setCurrentRow(this->currentRow() - 1);
            }
            return;
    }
}
