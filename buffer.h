#pragma once

#include <QByteArray>
#include <QFile>
#include <QTextEdit>
#include <QString>

class Editor;

class Buffer
{
public:
	Buffer();
	// Buffer creates a buffer targeting a given file.
	Buffer(QString filename);

	// read returns the content of the buffer. It reads the content from the file
	// on disk if has not been already done.
	QByteArray read();

	// save saves the file on disk.
	void save(Editor* editor);

	const QString& getFilename() { return this->filename; }

	// onLeave is called when the Window is leaving this Buffer (either to show
	// another or because we're closing the application for instance).
	void onLeave(Editor* editor);

	// onEnter is called when the window is starting to display this buffer.
	void onEnter(Editor* editor);

	// lastCursorPosition is the last cursor position when the user has either
	// moved to another buffer or closed the editor, etc.
	// Use QTextCursor::setPosition() to set it back to the QTextCursor and then
	// QTextEditr::setTextCursor() to restore it in the editor.
	int lastCursorPosition;

protected:

private:
	QString filename;

	bool readFromDisk;
	QByteArray data;
};
