#include <QFileInfo>
#include <QMessageBox>
#include <QTextCursor>
#include <QScrollBar>
#include <QSettings>

#include "buffer.h"
#include "editor.h"
#include "git.h"
#include "window.h"

#include "qdebug.h"

Buffer::Buffer(Editor* editor, QString name) :
    editor(editor),
    modified(false),
    filename(""),
    name(name),
    alreadyReadFromDisk(false),
    bufferType(BUFFER_TYPE_UNKNOWN) {
}

Buffer::Buffer(Editor* editor, QString name, QString filename) :
    editor(editor),
    modified(false),
    name(name),
    alreadyReadFromDisk(false),
    bufferType(BUFFER_TYPE_FILE) {
    // resolve the absolute path of this
    QFileInfo info(filename);
    // happens when the file does not exist
    if (info.canonicalFilePath() == "") {
        // we believe the filename to be canonical
        this->filename = filename;
    } else {
        this->filename = info.canonicalFilePath();
    }
}

Buffer::Buffer(Editor* editor, QString name, QByteArray data) :
    editor(editor),
    modified(false),
    filename(""),
    name(name),
    alreadyReadFromDisk(false),
    bufferType(BUFFER_TYPE_UNKNOWN) {
    this->data = data;
};

QByteArray Buffer::read() {
    if (alreadyReadFromDisk) {
        return this->data;
    }

    if (QFile::exists(this->filename)) {
        QFile file(this->filename);
        // warn when the file is larger than 10MB
        if (file.size() > 1024*1024*10) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Large file.");
            msgBox.setText("The file '" + this->filename + "' is " + QString::number(file.size()/1024/1024) +  "MB, do you confirm that you want to open it?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            if (msgBox.exec() == QMessageBox::Cancel) {
                file.close();
                return this->data;
            }
        }
        file.open(QIODevice::ReadOnly);
        this->data = file.readAll();
        file.close();
        this->alreadyReadFromDisk = true;
    }

    return this->data;
}

QByteArray Buffer::reload() {
    this->alreadyReadFromDisk = false;
    return this->read();
}

QString Buffer::getId() {

    // BUFFER_TYPE_FILE
    // ----------------

    if (this->bufferType == BUFFER_TYPE_FILE) {
        if (!this->filename.isEmpty()) {
            return this->filename;
        } else {
            qWarning() << "warn: empty filename for a BUFFER_TYPE_FILE";
            return this->name;
        }
    }

    // BUFFER_TYPE_GIT_*
    // -----------------

    QString rv;
    switch (this->bufferType) {
    case BUFFER_TYPE_GIT_BLAME:
        rv = "GIT BLAME - ";
        break;
    case BUFFER_TYPE_GIT_SHOW:
        rv = "GIT SHOW - ";
        break;
    case BUFFER_TYPE_GIT_DIFF:
        rv = "GIT DIFF - ";
        break;
    }

    // BUFFER_TYPE_COMMAND
    // -------------------

    if (this->bufferType == BUFFER_TYPE_COMMAND) {
        rv = "COMMAND - ";
    }

    // other kind of buffers
    // ---------------------

    rv += this->name;
    return rv;
}

// TODO(remy): while saving the buffer into a file, it should turns the buffer
// into a BUFFER_TYPE_FILE.
void Buffer::save(Window* window) {
    Q_ASSERT(this->editor != nullptr);

    QFile file(filename);

    file.open(QIODevice::Truncate | QIODevice::ReadWrite);
    if (this->editor->getBuffer() == this) {
        file.write(this->editor->toPlainText().toUtf8());
    } else {
        file.write(this->data);
    }
    file.close();

    if (this->postProcess()) {
        this->alreadyReadFromDisk = false;
        if (this->editor->getBuffer() == this) {
            // store some cursor / scroll positions
            QTextCursor cursor = this->editor->textCursor();
            int position = cursor.position();
            QScrollBar* vscroll = this->editor->verticalScrollBar();
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
            this->editor->ensureCursorVisible();
        } else {
            // refresh the data of this buffer
            this->read();
        }
    }

    this->modified = false;
    // TODO(remy): error management
}

bool Buffer::isGitTempFile() {
    return Git::isGitTempFile(this->filename);
}

bool Buffer::postProcess() {
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

void Buffer::refreshData(Window* window) {
    Q_ASSERT(window != NULL);
    Q_ASSERT(window->getEditor(this->getId()) != NULL);
    // store the last data from the currently displayed document in the buffer
    this->data = window->getEditor(this->getId())->document()->toPlainText().toUtf8();
}

void Buffer::onLeave() {
    Q_ASSERT(this->editor != nullptr);

    // store the last data from the document in the buffer
    this->data = this->editor->document()->toPlainText().toUtf8();

    QScrollBar* vscroll = this->editor->verticalScrollBar();

    // store last cursor position in settings
    QSettings settings("mehteor", "meh");
    settings.setValue("buffer/" + this->filename + "/cursor", editor->textCursor().position());
    settings.setValue("buffer/" + this->filename + "/vscroll", vscroll->value());
}

void Buffer::onClose() {
    Q_ASSERT(this->editor != nullptr);
    this->editor->removeOpenedState(this->filename);
}

void Buffer::onEnter() {
    Q_ASSERT(this->editor != nullptr);

    // TODO(remy): check whether the file has changed on disk? compare timestamp?

    // restore the text in the editor
    this->editor->setPlainText(this->read());

    // restore last cursor position, but do not do that for git messages
    if (this->isGitTempFile()) {
        return;
    }

    QSettings settings("mehteor", "meh");

    QTextCursor cursor = this->editor->textCursor();
    auto pos = settings.value("buffer/" + this->filename + "/cursor", 0).toInt();
    if (pos > this->editor->document()->characterCount()) {
        cursor.setPosition(this->editor->document()->characterCount() - 1);
    } else {
        cursor.setPosition(pos);
    }
    this->editor->setTextCursor(cursor);

    QScrollBar* vscroll = this->editor->verticalScrollBar();
    vscroll->setValue(settings.value("buffer/" + this->filename + "/vscroll", 0).toInt());
    this->editor->centerCursor();
}
