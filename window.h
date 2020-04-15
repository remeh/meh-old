#pragma once

#include <QTextEdit>

#include "buffer.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class Window : public QTextEdit
{
    Q_OBJECT
public:
    Window(QWidget *parent = nullptr);

	void setCurrentBuffer(Buffer *buffer);

	void setBlockCursor() { this->setCursorWidth(7); }
	void setLineCursor() { this->setCursorWidth(1); }

protected:
	void keyPressEvent(QKeyEvent *event);

private slots:

private:
	Buffer *currentBuffer;

};
