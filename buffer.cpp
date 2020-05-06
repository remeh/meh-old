#include <QFileInfo>
#include <QTextCursor>
#include <QScrollBar>
#include <QSettings>

#include "buffer.h"
#include "editor.h"

#include "qdebug.h"

Buffer::Buffer() :
    modified(false),
    readFromDisk(false) {
}

Buffer::Buffer(QString filename) :
    modified(false),
    readFromDisk(false) {
    // resolve the absolute path of this
    QFileInfo info(filename);
    this->filename = info.absoluteFilePath();
}

QByteArray Buffer::read() {
    if (readFromDisk) {
        return this->data;
    }

    QFile file(this->filename);
    // TODO(remy): it creates the file
    file.open(QIODevice::ReadOnly);
    this->data = file.readAll();
    file.close();
    this->readFromDisk = true;
    return this->data;
}

void Buffer::save(Editor* editor) {
    Q_ASSERT(editor != NULL);
    QFile file(filename);
    file.open(QIODevice::Truncate | QIODevice::ReadWrite);
    if (editor->getCurrentBuffer() == this) {
        file.write(editor->toPlainText().toUtf8());
    } else {
        file.write(this->data);
    }
    file.close();

    if (this->postProcess(editor)) {
        this->readFromDisk = false;
        if (editor->getCurrentBuffer() == this) {
            // refresh the data of this buffer, update the editor content
            QTextCursor cursor = editor->textCursor();
            int position = cursor.position();
            QScrollBar* vscroll = editor->verticalScrollBar();
            int value = vscroll->value();
            editor->setText(this->read());
            cursor.setPosition(position);
            editor->setTextCursor(cursor);
            vscroll->setValue(value);
            editor->ensureCursorVisible();
        } else {
            // refresh the data of this buffer
            this->read();
        }
    }

    this->modified = false;
    // TODO(remy): error management
}

bool Buffer::postProcess(Editor*) {
    if (this->filename.endsWith(".go")) {
        QProcess process;
        process.start("gofmt", QStringList() << "-l" << "-w" << this->filename);
        if (!process.waitForFinished(10000)) {
            qWarning() << "while running: gofmt";
        }
        return true;
    }
    return false;
}

void Buffer::onLeave(Editor* editor) {
    Q_ASSERT(editor != NULL);

    // store the last data from the document in the buffer
    this->data = editor->document()->toPlainText().toUtf8();

    QScrollBar* vscroll = editor->verticalScrollBar();

    // store last cursor position in settings
    QSettings settings("mehteor", "meh");
    settings.setValue("buffer/" + this->filename + "/cursor", editor->textCursor().position());
    settings.setValue("buffer/" + this->filename + "/vscroll", vscroll->value());
}

void Buffer::onEnter(Editor* editor) {
    Q_ASSERT(editor != NULL);

    // TODO(remy): check whether the file has changed on disk? compare timestamp?

    // restore the text in the editor
    editor->setText(this->read());

    // restore last cursor position
    QTextCursor cursor = editor->textCursor();
    QSettings settings("mehteor", "meh");
    cursor.setPosition(settings.value("buffer/" + this->filename + "/cursor", 0).toInt());
    editor->setTextCursor(cursor);
    QScrollBar* vscroll = editor->verticalScrollBar();
    vscroll->setValue(settings.value("buffer/" + this->filename + "/vscroll", 0).toInt());
    editor->ensureCursorVisible();
}
