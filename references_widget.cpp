#include <QGridLayout>
#include <QWidget>

#include "references_widget.h"
#include "window.h"

ReferencesWidget::ReferencesWidget(Window* window) :
    QWidget(window),
    window(window) {
    Q_ASSERT(window != nullptr);

    this->label = new QLabel(this);
    this->tree = new QTreeWidget(this);

    this->tree->setSortingEnabled(true);
    this->tree->setColumnCount(3);
    QStringList columns;
    columns << "File" << "Line #" << "Line";
    this->tree->setHeaderLabels(columns);

    this->setFont(Editor::getFont());
    this->setFocusPolicy(Qt::StrongFocus);

    this->layout = new QGridLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->addWidget(this->label);
    this->layout->addWidget(this->tree);
    this->setLayout(layout);
}

void ReferencesWidget::onKeyPressEvent(QKeyEvent* event) {
    this->keyPressEvent(event);
}

void ReferencesWidget::fitContent() {
    this->tree->setColumnWidth(1, 50);
    this->tree->resizeColumnToContents(0);
    if (this->tree->columnWidth(0) > 300) {
        this->tree->setColumnWidth(0, 300);
    }
}

void ReferencesWidget::sort(int column, Qt::SortOrder order) {
    if (this->tree) {
        this->tree->sortItems(column, order);
    }
}

void ReferencesWidget::show() {
    this->tree->show();
    this->label->setFocus();
    QWidget::show();
    this->label->setText("No results");
}

void ReferencesWidget::hide() {
    this->tree->clear();
    this->label->hide();
    this->tree->hide();
    data.clear();
    QWidget::hide();
}

void ReferencesWidget::clear() {
    this->label->setText("");
    this->tree->clear();
}

void ReferencesWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            this->window->closeGrep();
            this->window->getEditor()->setMode(MODE_NORMAL);
            return;
    }

    #ifdef Q_OS_MAC
        bool ctrl = event->modifiers() & Qt::MetaModifier;
    #else
        bool ctrl = event->modifiers() & Qt::ControlModifier;
    #endif

    switch (event->key()) {
        case Qt::Key_Return:
            {
                QTreeWidgetItem* currentItem = this->tree->currentItem();
                if (currentItem == nullptr) {
                    return;
                }
                QString filename = currentItem->text(0);
                QString lineNumber = currentItem->text(1);
                this->window->saveCheckpoint();
                QString filepath = currentItem->data(REF_WIDGET_DATA_ID, Qt::UserRole).toString();
                if (filepath.size() == 0) {
                    qDebug() << "error: currentItem->data(REF_WIDGET_DATA_ID, Qt::UserRole)) is empty";
                }
                this->window->setCurrentEditor(filepath);
                this->window->getEditor()->goToLine(lineNumber.toInt());
                this->window->getEditor()->setFocus();
                return;
            }
        case Qt::Key_N:
        case Qt::Key_J:
            if ((event->key() == Qt::Key_N && ctrl) || (event->key() == Qt::Key_J && !ctrl)) {
                QTreeWidgetItem* currentItem = this->tree->currentItem();
                if (currentItem == nullptr) {
                    currentItem = this->tree->topLevelItem(0);
                    this->tree->setCurrentItem(currentItem);
                    return;
                }
                currentItem = this->tree->itemBelow(currentItem);
                if (currentItem != nullptr) {
                    this->tree->setCurrentItem(currentItem);
                }
            } else if (event->key() == Qt::Key_J && ctrl) {
                QTreeWidgetItem* currentItem = this->tree->currentItem();
                if (currentItem != nullptr) {
                    currentItem->setExpanded(true);
                    this->tree->setCurrentItem(currentItem);
                }
            }
            return;
        case Qt::Key_Backspace:
        case Qt::Key_X:
            {
                QTreeWidgetItem* currentItem = this->tree->currentItem();
                if (currentItem != nullptr) {
                    delete currentItem;
                }
            }
            return;
        case Qt::Key_P:
        case Qt::Key_K:
            if ((event->key() == Qt::Key_P && ctrl) || (event->key() == Qt::Key_K && !ctrl)) {
                QTreeWidgetItem* currentItem = this->tree->currentItem();
                if (currentItem == nullptr) {
                    currentItem = this->tree->topLevelItem(0);
                    this->tree->setCurrentItem(currentItem);
                    return;
                }
                currentItem = this->tree->itemAbove(currentItem);
                if (currentItem != nullptr) {
                    this->tree->setCurrentItem(currentItem);
                }
            } else if (event->key() == Qt::Key_K && ctrl) {
                QTreeWidgetItem* currentItem = this->tree->currentItem();
                if (currentItem != nullptr) {
                    currentItem->setExpanded(false);
                    this->tree->setCurrentItem(currentItem);
                }
            }
            return;
    }
}

void ReferencesWidget::setLabelText(QString text) {
    if (this->label != nullptr) {
        this->label->setText(text);
    }
}

void ReferencesWidget::insert(const QString& filepath, const QString& lineNumber, const QString& text) {
    QFileInfo info(filepath);
    QStringList parts; parts << info.fileName() << lineNumber << text;
    if (data.contains(filepath)) {
        QTreeWidgetItem* item = new QTreeWidgetItem(this->data[filepath], parts);
        item->setData(REF_WIDGET_DATA_ID, Qt::UserRole, filepath);
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(this->tree, parts);
        item->setData(REF_WIDGET_DATA_ID, Qt::UserRole, filepath);
        this->data[filepath] = item;
    }
}
