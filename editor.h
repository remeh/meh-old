#pragma once

#include <QMap>
#include <QVector>
#include <QTextEdit>

#include "buffer.h"
#include "mode.h"

class Window;

class Editor : public QTextEdit
{
    Q_OBJECT
public:
    Editor(Window* window);
	~Editor();

	// setCurrentBuffer sets the editor to use the given buffer.
	void setCurrentBuffer(Buffer* buffer);
	// getCurrentBuffer retuns the currently used buffer.
	Buffer* getCurrentBuffer() { return this->currentBuffer; }
	// selectBuffer uses an already opened buffer and set it as the active one.
	void selectBuffer(const QString& filename);
	// hasBuffer returns true if a buffer has already been loaded.
	bool hasBuffer(const QString& filename);

	void setMode(int mode, QString command = "");
	int getMode() { return this->mode; }

	void setSubMode(int subMode);
	int getSubMode() { return this->subMode; }

	void setBlockCursor() { this->setCursorWidth(7); } // FIXME(remy):
	void setLineCursor() { this->setCursorWidth(1); }

	// goToLine moves the cursor to a given position in the buffer.
	void goToLine(int lineNumber);

	// save saves the current buffer.
	void save() { this->currentBuffer->save(this); }

protected:
	void keyPressEvent(QKeyEvent* event);

private slots:

private:
	// keyPressEventNormal handles this event in normal mode.
	// ctrl is Control on Linux, Windows but is Command on macOS.
	void keyPressEventNormal(QKeyEvent* event, bool ctrl, bool shift);
	void keyPressEventSubMode(QKeyEvent* event, bool ctrl, bool shift);

	// currentLineIndent returns the current line indentation.
	QString currentLineIndent();

	// ----------------------

	Window* window;

	// currentBuffer is the currently visible buffer. Note that it is not part
	// of the buffers map.
	Buffer* currentBuffer;

	// buffers is the currently loaded buffers. Note that it doesn't contain
	// the currentBuffer. It is the owner of the buffers.
	QMap<QString, Buffer*> buffers;

	// bufferPos can be used to know the order of usage of the buffers.
	QVector<QString> buffersPos;

	int mode;
	int subMode;
};
