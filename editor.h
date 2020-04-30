#pragma once

#include <QChar>
#include <QColor>
#include <QLabel>
#include <QMap>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QVector>
#include <QTextEdit>
#include <QTimer>

#include "buffer.h"
#include "fileslookup.h"
#include "mode.h"
#include "syntax.h"

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
    // selectOrCreateBuffer uses an already opened buffer and set it as the active one,
    // if this buffer doesn't exist (file not already loaded) it creates it.
    void selectOrCreateBuffer(const QString& filename);
    // hasBuffer returns true if a buffer has already been loaded.
    bool hasBuffer(const QString& filename);

    void setMode(int mode, QString command = "");
    int getMode() { return this->mode; }

    void setSubMode(int subMode);
    int getSubMode() { return this->subMode; }

    void setBlockCursor() { this->setCursorWidth(7); } // FIXME(remy):
    void setMidCursor() { this->setCursorWidth(4); } // FIXME(remy):
    void setLineCursor() { this->setCursorWidth(1); }

    // goToLine moves the cursor to a given position in the buffer.
    void goToLine(int lineNumber);

    // goToOccurrence goes to the next occurence of string if any, previous one
    // if backward is set.
    void goToOccurrence(const QString& string, bool backward);

    // deleteCurrentLine removes the current line of the buffer.
    void deleteCurrentLine();

    // getWordUnderCursor returns the word under the cursor if any.
    QString getWordUnderCursor();

    // save saves the current buffer.
    void save() { this->currentBuffer->save(this); }

    // called by the window when it is resized.
    void onWindowResized(QResizeEvent*);

    // removeIndentation remove one layer of indentation on the current line.
    void removeIndentation();

protected:
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private slots:
    void onSelectionChanged();
    void onCursorPositionChanged();
    void onTriggerSelectionHighlight();

private:
    // keyPressEventNormal handles this event in normal mode.
    // ctrl is Control on Linux, Windows but is Command on macOS.
    void keyPressEventNormal(QKeyEvent* event, bool ctrl, bool shift);
    void keyPressEventVisual(QKeyEvent* event, bool ctrl, bool shift);
    void keyPressEventSubMode(QKeyEvent* event, bool ctrl, bool shift);

    // currentLineIndent returns the current line indentation.
    QString currentLineIndent();

    // currentLineIsOnlyWhitespaces return -1 if this is wrong and returns how
    // many whitespaces (' ' or '\t') are composing this line.
    int currentLineIsOnlyWhitespaces();

    // currentLineLastChar returns the last char of the line.
    // moveUp should be used to move to the line above first. Used for O in normal.
    QChar currentLineLastChar(bool moveUp);

    // findNextOneInCurrentLine returns the distance to the next occurence of the
    // given char in the current line. The distance is from the current cursor position.
    int findNextOneInCurrentLine(QChar c);

    // findPreviousOneInCurrentLine returns the distance to the previous occurence of
    // the given char in the current line. The distance is from the current cursor position.
    int findPreviousOneInCurrentLine(QChar c);

    // ----------------------

    QTimer* selectionTimer;
    QLabel* modeLabel;
    QLabel* lineLabel;

    Window* window;

    Syntax* syntax;

    // currentBuffer is the currently visible buffer. Note that it is not part
    // of the buffers map.
    Buffer* currentBuffer;

    // buffers is the currently loaded buffers. Note that it doesn't contain
    // the currentBuffer. It is the owner of the buffers.
    QMap<QString, Buffer*> buffers;

    // bufferPos can be used to know the order of usage of the buffers.
    QVector<QString> buffersPos;

    // mode is the currently used mode. See mode.h
    int mode;

    // subMode is the currently used mode. See mode.h
    int subMode;

    // eightCharsX is the X position where the eighty chars line must be drawn.
    int eightyCharsX;

    QColor highlightedLine;
};

