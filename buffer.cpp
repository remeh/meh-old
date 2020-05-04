#include <QFileInfo>
#include <QSettings>

#include "buffer.h"
#include "editor.h"

#include "qdebug.h"

Buffer::Buffer() :
    readFromDisk(false) {
}

Buffer::Buffer(QString filename) :
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
    file.open(QIODevice::ReadWrite);

    this->data = file.readAll();
    this->readFromDisk = true;
    return this->data;
}

void Buffer::save(Editor* editor) {
    Q_ASSERT(editor != NULL);
    QFile file(filename);
    file.open(QIODevice::Truncate | QIODevice::ReadWrite);
    file.write(editor->toPlainText().toUtf8());
    this->modified = false;

    // TODO(remy): error management
}

void Buffer::onLeave(Editor* editor) {
    Q_ASSERT(editor != NULL);

    // store the last data from the document in the buffer
    this->data = editor->document()->toPlainText().toUtf8();

    // store last cursor position in settings
    QSettings settings("mehteor", "meh");
    settings.setValue("buffer/" + this->filename, editor->textCursor().position());
}

void Buffer::onEnter(Editor* editor) {
    Q_ASSERT(editor != NULL);

    // TODO(remy): check whether the file has changed on disk? compare timestamp?

    // restore the text in the editor
    editor->setText(this->read());

    // restore last cursor position
    auto cursor = editor->textCursor();
    QSettings settings("mehteor", "meh");
    cursor.setPosition(settings.value("buffer/" + this->filename, 0).toInt());
    editor->setTextCursor(cursor);
}
