#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>

#include "qdebug.h"

#include "fileslookup.h"
#include "mode.h"
#include "window.h"

FilesLookup::FilesLookup(Window* window) :
    QWidget(window),
    window(window) {
    Q_ASSERT(window != nullptr);

    this->edit = new QLineEdit(this);
    this->label = new QLabel(this);
    this->list = new QListWidget(this);
    this->list->setSortingEnabled(true);

    this->setFocusPolicy(Qt::StrongFocus);

    this->layout = new QGridLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->addWidget(this->edit);
    this->layout->addWidget(this->label);
    this->layout->addWidget(this->list);
    this->setLayout(layout);

    connect(this->edit, &QLineEdit::textChanged, this, &FilesLookup::onEditChanged);
}

void FilesLookup::onEditChanged() {
    this->resetFiltered();
    this->filter();
    this->refreshList();
    this->list->setCurrentRow(0);
}

void FilesLookup::showFiles() {
    this->base = "";
    this->edit->setText("");
    this->lookupDir(this->window->getBaseDir());
    this->show();
}

void FilesLookup::showBuffers() {
    this->base = "";
    this->edit->setText("");
    this->lookupBuffers();
    this->show();
}

void FilesLookup::show() {
    this->edit->show();
    this->label->show();
    this->list->show();
    this->edit->setFocus();
    this->refreshList();
    QWidget::show();
}

void FilesLookup::hide() {
    this->edit->hide();
    this->label->hide();
    this->list->hide();
    QWidget::hide();
}

void FilesLookup::lookupBuffers() {
    this->filenames.clear();
    this->filteredFiles.clear();
    this->directories.clear();
    this->filteredDirs.clear();

    QList<Buffer*> buffers = this->window->getEditor()->getBuffers().values();
    for (int i = 0; i < buffers.size(); i++) {
        this->filenames.insert(buffers.at(i)->getFilename());
        this->filteredFiles.insert(buffers.at(i)->getFilename());
    }
}

void FilesLookup::lookupDir(QString filepath) {
    this->filenames.clear();
    this->filteredFiles.clear();
    this->directories.clear();
    this->filteredDirs.clear();

    if (filepath.endsWith("/")) {
        this->base += filepath;
    } else {
        this->base += filepath + "/";
    }
    this->label->setText(this->base);
    QDir dir(this->base);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++) {
        QFileInfo info = list.at(i);
        if (info.isDir()) {
            QString filename = info.fileName();
            if (filename == ".") { continue; }
            this->directories.insert(filename);
            this->filteredDirs.insert(filename);
        } else {
            this->filenames.insert(info.fileName());
            this->filteredFiles.insert(info.fileName());
        }
    }
}

bool FilesLookup::openSelection() {
    QListWidgetItem* item = this->list->currentItem();
    if (item == nullptr) {
        qDebug() << "item == nullptr in FilesLookup::keyPressEvent";
    }

    QFileInfo info(this->base + item->text());
    if (info.isFile()) {
        this->window->getEditor()->selectOrCreateBuffer(info.absoluteFilePath());
        return true;
    } else {
        this->lookupDir(item->text());
        this->edit->setText("");
        this->refreshList();
        return false;
    }
}

void FilesLookup::keyPressEvent(QKeyEvent* event) {
    #ifdef Q_OS_MAC
        bool ctrl = event->modifiers() & Qt::MetaModifier;
    #else
        bool ctrl = event->modifiers() & Qt::ControlModifier;
    #endif

    switch (event->key()) {
        case Qt::Key_Escape:
            this->window->closeList();
            this->window->getEditor()->setMode(MODE_NORMAL);
            return;

        // TODO(remy): we probably want to have a custom QLineEdit to be able
        // to properly override its keyPressEvent to be able to use Key_Down
        // to focus on the list, backspace to clear all the text.

        case Qt::Key_N:
            if (ctrl) {
                if (this->list->currentRow() == this->list->count() - 1) {
                    this->list->setCurrentRow(0);
                } else {
                    this->list->setCurrentRow(this->list->currentRow() + 1);
                }
            }
            return;

        case Qt::Key_P:
            if (ctrl) {
                if (this->list->currentRow() <= 0) {
                    this->list->setCurrentRow(this->list->count() - 1);
                } else {
                    this->list->setCurrentRow(this->list->currentRow() - 1);
                }
            }
            return;

        case Qt::Key_Return:
            {
                if (this->list->currentItem() == nullptr) {
                    return;
                }
                bool close = this->openSelection();
                if (close) {
                    this->window->closeList();
                }
            }
            return;
    }
}

void FilesLookup::resetFiltered() {
    this->filteredDirs.clear();
    this->filteredFiles.clear();

    auto it = this->directories.begin();
    while (it != this->directories.end()) {
        this->filteredDirs.insert(*it);
        ++it;
    }

    it = this->filenames.begin();
    while (it != this->filenames.end()) {
        this->filteredFiles.insert(*it);
        ++it;
    }

}

void FilesLookup::filter() {
    QString string = this->edit->text();
    auto it = this->filteredDirs.constBegin();
    while (it != this->filteredDirs.constEnd()) {
        // TODO(remy): fuzzy search
        if (!(it->contains(QRegularExpression(string)))) {
            it = this->filteredDirs.erase(it);
        } else {
            ++it;
        }
    }

    it = this->filteredFiles.begin();
    while (it != this->filteredFiles.end()) {
        // TODO(remy): fuzzy search
        if (!it->contains(QRegularExpression(string))) {
            it = this->filteredFiles.erase(it);
        } else {
            ++it;
        }
    }
}

// TODO(remy): a performance improvements would be to not empty it and refill it
// every time but just removing the filtered entries.
void FilesLookup::refreshList() {
    this->list->clear();
    auto it = this->filteredDirs.begin();
    while (it != this->filteredDirs.end()) {
        QIcon icon = QIcon(":/res/directory-in.png");
        if (*it == "..") {
            icon = QIcon(":/res/directory-out.png");
        }
        new QListWidgetItem(icon, *it, this->list);
        ++it;
    }
    it = this->filteredFiles.begin();
    while (it != this->filteredFiles.end()) {
        QIcon icon = QIcon(":/res/plus.png");
        if (this->window->getEditor()->hasBuffer(*it)) {
            icon = QIcon(":/res/edit.png");
        }
        new QListWidgetItem(icon, *it, this->list);
        ++it;
    }
}
