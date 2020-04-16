#pragma once

#include <QTextEdit>

#include "buffer.h"
#include "mode.h"

class Editor : public QTextEdit
{
    Q_OBJECT
public:
    Editor(QWidget *parent = nullptr);

	void setCurrentBuffer(Buffer *buffer);
	Buffer* getCurrentBuffer() { return this->currentBuffer; }

	void setMode(int mode);
	int getMode() { return this->mode; }

	void setBlockCursor() { this->setCursorWidth(7); }
	void setLineCursor() { this->setCursorWidth(1); }

	// TODO(remy): use this->overwriteMode for the replace mode

protected:
	void keyPressEvent(QKeyEvent *event);

private slots:

private:
	Buffer* currentBuffer;

	int mode;
};
