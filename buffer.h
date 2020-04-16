#pragma once

#include <QByteArray>
#include <QFile>
#include <QTextEdit>
#include <QString>

class Buffer
{
public:
	Buffer();
	// Buffer creates a buffer targeting a given file.
	Buffer(QString filename);

	// readFile reads the file on disk and returns its content.
	QByteArray readFile();

	// onLeave is called when the Window is leaving this Buffer (either to show
	// another or because we're closing the application for instance).
	void onLeave(const QTextEdit& editor);

	int lastCursorPosition() { return this->_lastCursorPosition; }
	void setLastCursorPosition(int v) { this->_lastCursorPosition = v; }

protected:

private:
	QString filename;

	// _lastCursorPosition is the last cursor position when the user has either
	// moved to another buffer or closed the editor, etc.
	// Use QTextCursor::setPosition() to set it back to the QTextCursor and then
	// QTextEditr::setTextCursor() to restore it in the editor.
	int _lastCursorPosition;
};
