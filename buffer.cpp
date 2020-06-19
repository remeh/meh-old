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

    if (QFile::exists(this->filename)) {
        QFile file(this->filename);
        file.open(QIODevice::ReadOnly);
        this->data = file.readAll();
        file.close();
        this->readFromDisk = true;
    }

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
            // store some cursor / scroll positions
            QTextCursor cursor = editor->textCursor();
            int position = cursor.position();
            QScrollBar* vscroll = editor->verticalScrollBar();
            int value = vscroll->value();

            // reset the content by re-reading the text
            // we do it this way instead of a simple setText in order
            // to keep the editor history.
            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.deleteChar();
            cursor.insertText(this->read());
            cursor.endEditBlock();

            // reposition the scroll and the cursor
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

bool Buffer::isGitTempFile() {
    return filename.endsWith(".git/COMMIT_EDITMSG") || filename.endsWith(".git/MERGE_MSG");
}

bool Buffer::postProcess(Editor*) {
    if (this->filename.endsWith(".go")) {
        QProcess process;
        process.start("gofmt", QStringList() << "-l" << "-w" << this->filename);
        if (!process.waitForFinished(10000)) {
            qWarning() << "while running: gofmt";
        }
        process.start("goimports", QStringList() << "-w" << this->filename);
        if (!process.waitForFinished(10000)) {
            qWarning() << "while running: goimports";
        }
        return true;
    }
    if (this->filename.endsWith(".zig")) {
        QProcess process;
        process.start("zig", QStringList() << "fmt" << this->filename);
        if (!process.waitForFinished(10000)) {
            qWarning() << "while running: zig fmt";
        }
        return true;
    }
    return false;
}

void Buffer::refreshData(Editor* editor) {
    Q_ASSERT(editor != NULL);
    // store the last data from the document in the buffer
    this->data = editor->document()->toPlainText().toUtf8();
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
    editor->setPlainText(this->read());

    // restore last cursor position, but do not do that for git messages
    if (!this->isGitTempFile()) {
        QTextCursor cursor = editor->textCursor();
        QSettings settings("mehteor", "meh");
        cursor.setPosition(settings.value("buffer/" + this->filename + "/cursor", 0).toInt());
        editor->setTextCursor(cursor);
        QScrollBar* vscroll = editor->verticalScrollBar();
        vscroll->setValue(settings.value("buffer/" + this->filename + "/vscroll", 0).toInt());
        editor->ensureCursorVisible();
    }
}
