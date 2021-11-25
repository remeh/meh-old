#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFrame>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>

#include "qdebug.h"

#include "exec.h"
#include "fileslookup.h"
#include "mode.h"
#include "window.h"

FilesLookup::FilesLookup(Window* window) :
    QFrame(window),
    window(window) {
    Q_ASSERT(window != nullptr);

    this->edit = new QLineEdit(this);
    this->label = new QLabel(this);
    this->list = new QListWidget(this);
    this->list->setSortingEnabled(true);
    this->edit->setFont(Editor::getFont());
    this->label->setFont(Editor::getFont());
    this->list->setFont(Editor::getFont());
    this->setFont(Editor::getFont());

    this->setFocusPolicy(Qt::StrongFocus);

    this->layout = new QGridLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->addWidget(this->edit);
    this->layout->addWidget(this->label);
    this->layout->addWidget(this->list);
    this->setLayout(layout);

    connect(this->edit, &QLineEdit::textChanged, this, &FilesLookup::onEditChanged);
    connect(this->list, &QListWidget::itemDoubleClicked, this, &FilesLookup::onItemDoubleClicked);

}

void FilesLookup::onEditChanged() {
    this->resetFiltered();
    this->filter();
    this->refreshList();
    this->list->setCurrentRow(0);
}

void FilesLookup::onItemDoubleClicked() {
    this->openSelection();
    this->hide();
}

void FilesLookup::showFiles() {
    this->base = "";
    this->edit->setText("");
    this->lookupDir(this->window->getBaseDir());
    this->show();
}

void FilesLookup::showBuffers() {
    this->base = this->window->getBaseDir();
    this->edit->setText("");
    this->lookupBuffers();
    this->show();
}

void FilesLookup::show() {
    int winWidth = this->window->size().width();
    int winHeight = this->window->size().height();

    int popupWidth = winWidth  - winWidth/3;
    int popupHeight = winHeight / 2;
    this->resize(popupWidth, popupHeight);
    this->move(winWidth / 2 - (winWidth/3), 120);

    this->refreshList();
    this->edit->setFocus();
    QWidget::show();
    QWidget::raise();
}

void FilesLookup::hide() {
    QWidget::hide();
}

void FilesLookup::lookupBuffers() {
    this->filenames.clear();
    this->filteredFiles.clear();
    this->directories.clear();
    this->filteredDirs.clear();
    this->buffers.clear();
    this->filteredBuffers.clear();

    this->list->setSortingEnabled(false);

    QList<Editor*> editors = this->window->getEditors();
    for (int i = 0; i < editors.size(); i++) {
        Buffer* buffer = editors.at(i)->getBuffer();
        QString value = buffer->getId();

        // remove basedir of filenames
        if (value.startsWith(this->window->getBaseDir())) {
            value = value.remove(0, this->window->getBaseDir().size());
            if (value.startsWith("/")) {
                value = value.remove(0, 1);
            }
        }

        if (buffer->getType() == BUFFER_TYPE_FILE) {
            this->filenames.insert(value);
            this->filteredFiles.insert(value);
        } else {
            this->buffers.insert(value);
            this->filteredBuffers.insert(value);
        }
    }
}

void FilesLookup::lookupDir(QString filepath) {
    this->filenames.clear();
    this->filteredFiles.clear();
    this->directories.clear();
    this->filteredDirs.clear();
    this->buffers.clear();
    this->filteredBuffers.clear();

    this->list->setSortingEnabled(true);

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

void FilesLookup::showList(QList<QString> files) {
    this->base = this->window->getBaseDir();

    this->filenames.clear();
    this->filteredFiles.clear();
    this->directories.clear();
    this->filteredDirs.clear();
    this->buffers.clear();
    this->filteredBuffers.clear();

    this->list->setSortingEnabled(true);

    for (QString file : files) {
        QFileInfo info = QFileInfo(file);
        if (info.isDir()) {
            this->directories.insert(file);
            this->filteredDirs.insert(file);
        } else {
            this->filenames.insert(file);
            this->filteredFiles.insert(file);
        }
    }

    this->show();
    this->edit->setText("");
}

bool FilesLookup::openSelection() {
    QListWidgetItem* item = this->list->currentItem();
    if (item == nullptr) {
        qDebug() << "item == nullptr in FilesLookup::keyPressEvent";
        return false;
    }

    QString id = item->data(FILESLOOKUP_DATA_ID).toString();
    QString type = item->data(FILESLOOKUP_DATA_TYPE).toString();

    if (type == "file") {
        this->window->saveCheckpoint();
        this->window->setCurrentEditor(id);
        return true;
    } else if (type == "buffer") {
        this->window->saveCheckpoint();
        this->window->setCurrentEditor(id);
        return true;
    } else if (type == "directory") {
        this->lookupDir(id);
        this->edit->setText("");
        this->refreshList();
        return false;
    }

    return false;
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
                const QString& text = this->edit->text();
                if (text.startsWith(":!fd ") || text.startsWith(":fd ")) {
                    QString str = this->edit->text().remove(0, text.indexOf(" ")+1);
                    this->window->getExec()->start(this->window->getBaseDir(), QStringList() << "fd" << str);
                    this->label->setText(this->edit->text());
                    return;
                }
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
    this->filteredBuffers.clear();

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

    it = this->buffers.begin();
    while (it != this->buffers.end()) {
        this->filteredBuffers.insert(*it);
        ++it;
    }
}

void FilesLookup::filter() {
    QString string = this->edit->text();
    auto it = this->filteredDirs.constBegin();
    while (it != this->filteredDirs.constEnd()) {
        if (!(it->contains(QRegularExpression(string)))) {
            it = this->filteredDirs.erase(it);
        } else {
            ++it;
        }
    }

    it = this->filteredFiles.begin();
    while (it != this->filteredFiles.end()) {
        if (!it->contains(QRegularExpression(string))) {
            it = this->filteredFiles.erase(it);
        } else {
            ++it;
        }
    }

    it = this->filteredBuffers.begin();
    while (it != this->filteredBuffers.end()) {
        if (!it->contains(QRegularExpression(string))) {
            it = this->filteredBuffers.erase(it);
        } else {
            ++it;
        }
    }
}

// TODO(remy): a performance improvements would be to not empty it and refill it
// every time but just removing the filtered entries.
void FilesLookup::refreshList() {
    this->list->clear();
    QSet<QString>::iterator it = this->filteredDirs.begin();
    while (it != this->filteredDirs.end()) {
        QIcon icon = QIcon(":/res/directory-in.png");
        if (*it == "..") {
            icon = QIcon(":/res/directory-out.png");
        }
        QListWidgetItem* item = new QListWidgetItem(icon, *it, this->list);
        item->setData(FILESLOOKUP_DATA_TYPE, "directory");
        item->setData(FILESLOOKUP_DATA_ID, *it);
        item->setText(*it);
        ++it;
    }
    it = this->filteredFiles.begin();
    while (it != this->filteredFiles.end()) {
        QString fullPath = *it;
        if (!(it->startsWith("/"))) {
            fullPath = this->base + *it;
        }
        QFileInfo fi(fullPath);
        fullPath = fi.canonicalFilePath();

        QIcon icon = QIcon(":/res/plus.png");
        if (this->window->hasBuffer(fullPath)) {
            icon = QIcon(":/res/edit.png");
        }
        QListWidgetItem* item = new QListWidgetItem(icon, *it, this->list);
        item->setData(FILESLOOKUP_DATA_TYPE, "file");
        item->setData(FILESLOOKUP_DATA_ID, fullPath);
        item->setText(*it);
        ++it;
    }
    it = this->filteredBuffers.begin();
    while (it != this->filteredBuffers.end()) {
        QListWidgetItem* item = new QListWidgetItem(QIcon(":/res/edit.png"), *it, this->list);
        item->setData(FILESLOOKUP_DATA_TYPE, "buffer");
        item->setData(FILESLOOKUP_DATA_ID, *it);
        item->setText(*it);
        ++it;
    }

    // now, select the first entry which is not ..
    if (this->list->count() == 1) {
        this->list->item(0)->setSelected(true);
        this->list->setCurrentRow(0);
    } else if (this->list->count() > 1) {
        this->list->item(1)->setSelected(true);
        this->list->setCurrentRow(1);
    }
}
