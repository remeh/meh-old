#include "buffer.h"
#include "editor.h"

Buffer::Buffer() :
	lastCursorPosition(0),
	readFromDisk(false) {
}

Buffer::Buffer(QString filename) :
	lastCursorPosition(0),
	filename(filename),
	readFromDisk(false) {
}

QByteArray Buffer::read() {
	if (readFromDisk) {
		return this->data;
	}

	QFile file(filename);
	if (!file.exists()) {
		// TODO(remy): doesn't exist.
	}
	file.open(QIODevice::ReadWrite);

	this->data = file.readAll();
	this->readFromDisk = true;
	return this->data;
}

void Buffer::save() {
	// TODO(remy): imlement me
}

void Buffer::onLeave(Editor* editor) {
	Q_ASSERT(editor != NULL);

	// store the last cursor position
	this->lastCursorPosition = editor->textCursor().position();

	// TODO(remy): implement me.
}

void Buffer::onEnter(Editor* editor) {
	Q_ASSERT(editor != NULL);

	// TODO(remy): check whether the file has changed on disk? compare timestamp?

	// restore the text in the editor
	editor->setText(this->read());

	// restore last cursor position
	auto cursor = editor->textCursor();
	cursor.setPosition(this->lastCursorPosition);
	editor->setTextCursor(cursor);

	// TODO(remy): implement me.
}
