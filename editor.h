#pragma once

#include <QChar>
#include <QColor>
#include <QFocusEvent>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>
#include <QVector>

#include "buffer.h"
#include "fileslookup.h"
#include "lsp.h"
#include "lsp_manager.h"
#include "mode.h"
#include "syntax.h"
#include "tasks.h"

class Window;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    Editor(Window* window);
    ~Editor();

    // buffer manipulation
    // -------------------

    // setCurrentBuffer sets the editor to use the given buffer.
    void setCurrentBuffer(Buffer* buffer);

    // getCurrentBuffer retuns the currently used buffer.
    Buffer* getCurrentBuffer() { return this->currentBuffer; }

    // selectOrCreateBuffer uses an already opened buffer and set it as the active one,
    // if this buffer doesn't exist (file not already loaded) it creates it.
    void selectOrCreateBuffer(const QString& filename);

    // closeCurrentBuffer closes the current buffer.
    void closeCurrentBuffer();

    // hasBuffer returns true if a buffer has already been loaded.
    bool hasBuffer(const QString& filename);

    // save saves the current buffer.
    void save();

    // saveAll saves all the loaded buffers.
    void saveAll();

    // modifiedBuffers returns a list of the loaded and modified buffers that
    // would need to be stored on disk.
    QStringList modifiedBuffers();

    QMap<QString, Buffer*>& getBuffers() { return this->buffers; }

    // mode
    // ----

    void setMode(int mode, QString command = "");
    int getMode() { return this->mode; }

    void setSubMode(int subMode);
    int getSubMode() { return this->subMode; }

    void setBlockCursor() { this->setCursorWidth(7); } // FIXME(remy):
    void setMidCursor() { this->setCursorWidth(4); } // FIXME(remy):
    void setLineCursor() { this->setCursorWidth(1); }

    // movements
    // ---------

    // goToLine moves the cursor to a given position in the buffer.
    void goToLine(int lineNumber);

    // goToColumn moves the cursor to the given column.
    void goToColumn(int column);

    // goToOccurrence goes to the next occurence of string if any, previous one
    // if backward is set.
    void goToOccurrence(const QString& string, bool backward);

    void up();
    void down();
    void left();
    void right();

    int currentLineNumber();
    int currentColumn();

    // text manipulation
    // -----------------

    // deleteCurrentLine removes the current line of the buffer.
    void deleteCurrentLine();

    // insertNewLine insert a new line either under, or above the current one
    // and takes care of adding the proper indentation.
    void insertNewLine(bool above);

    // getWordUnderCursor returns the word under the cursor if any.
    QString getWordUnderCursor();

    // removeIndentation removes one level of indentation on the line of the given cursor..
    void removeIndentation(QTextCursor cursor);

    // insertIndentation adds one level of indentation on the line of the given cursor.
    void insertIndentation(QTextCursor cursor);

    // auto-complete
    // -------------

    // TODO(remy): comment me
    void setCompleter(const QStringList& completer);
    // XXX(remy):
    void autocomplete();
    void applyAutocomplete(const QString& base, const QString& word);

    // lspInterpret is called by the LSP server to let the Editor interpret
    // the result.
    void lspInterpret(QByteArray data);

    LSPManager lspManager;

    // widget related
    // --------------

    // called by the window when it is resized.
    void onWindowResized(QResizeEvent*);

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();

protected:
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private slots:
    void onSelectionChanged();
    void onCursorPositionChanged();
    void onTriggerSelectionHighlight();
    void onTriggerLspRefresh();
    void onChange(bool changed);
    void onContentsChange(int position, int charsRemoved, int charsAdded);
    void onUpdateLineNumberAreaWidth(int newBlockCount);
    void onUpdateLineNumberArea(const QRect &rect, int dy);

private:
    // keyPressEventNormal handles this event in normal mode.
    // ctrl is Control on Linux, Windows but is Command on macOS.
    void keyPressEventNormal(QKeyEvent* event, bool ctrl, bool shift);
    void keyPressEventVisual(QKeyEvent* event, bool ctrl, bool shift);
    void keyPressEventVisualLine(QKeyEvent* event, bool ctrl, bool shift);
    void keyPressEventLeaderMode(QKeyEvent* event, bool ctrl, bool shift);
    void keyPressEventSubMode(QKeyEvent* event, bool ctrl, bool shift);

    // currentLineIndent returns the current line indentation.
    QString currentLineIndent();

    // toggleComments comments or uncomments the given blocks.
    void toggleComments(QList<QTextBlock> blocks, const QString& commentChars);

    // selectedBlocks returns all the block contained in the current selection.
    QList<QTextBlock> selectedBlocks();

    // currentLineIsOnlyWhitespaces return -1 if this is wrong and returns how
    // many whitespaces (' ' or '\t') are composing this line.
    int currentLineIsOnlyWhitespaces();

    // currentLineLastChar returns the last char of the line.
    QChar currentLineLastChar();

    // findNextOneInCurrentLine returns the distance to the next occurence of the
    // given char in the current line. The distance is from the current cursor position.
    int findNextOneInCurrentLine(QChar c);

    // findPreviousOneInCurrentLine returns the distance to the previous occurence of
    // the given char in the current line. The distance is from the current cursor position.
    int findPreviousOneInCurrentLine(QChar c);

    // currentBufferExtension returns the extension of the current file.
    // Returns an empty string if the buffer has no extension.
    QString currentBufferExtension();

    // leaderModeSelectSubMode sets the subMode we should switch in depending
    // of the current buffer.
    void leaderModeSelectSubMode();

    // ----------------------

    TasksPlugin *tasksPlugin;

    // ----------------------

    QTimer* selectionTimer;
    QTimer* lspRefreshTimer;
    QLabel* modeLabel;
    QLabel* lineLabel;
    QLabel* modifiedLabel;
    QListWidget* currentCompleter;
    QWidget* lineNumberArea;

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
    int hundredTwentyCharsX;

    QColor highlightedLine;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(Editor *editor) : QWidget(editor), editor(editor) {}

    QSize sizeHint() const override {
        return QSize(this->editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        this->editor->lineNumberAreaPaintEvent(event);
    }

private:
    Editor *editor;
};
