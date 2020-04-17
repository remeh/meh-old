#pragma once

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

	void setCurrentBuffer(Buffer* buffer);
	Buffer* getCurrentBuffer() { return this->currentBuffer; }

	void setMode(int mode);
	int getMode() { return this->mode; }

	void setBlockCursor() { this->setCursorWidth(7); }
	void setLineCursor() { this->setCursorWidth(1); }

	// TODO(remy): use this->overwriteMode for the replace mode

protected:
	void keyPressEvent(QKeyEvent* event);

private slots:

private:
	Window* window;

	// keyPressEventNormal handles this event in normal mode.
	// ctrl is Control on Linux, Windows but is Command on macOS.
	void keyPressEventNormal(QKeyEvent* event, bool ctrl, bool shift);

	Buffer* currentBuffer;
	int mode;
};
