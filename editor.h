#pragma once

#include <QChar>
#include <QColor>
#include <QFocusEvent>
#include <QFont>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTimer>
#include <QVector>

#include "buffer.h"
#include "fileslookup.h"
#include "lsp.h"
#include "lsp_manager.h"
#include "mode.h"
#include "references_widget.h"
#include "statusbar.h"
#include "syntax.h"
#include "tasks.h"

class Window;
class LineNumberArea;

class Checkpoint {
    public:
        Checkpoint(const QString& filename, int position) : filename(filename), position(position) {}
        QString filename;
        int position;
};

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

    // goToLine moves the cursor to a given position in the buffer
    // end centers the scroll on the cursor.
    void goToLine(int lineNumber);

    // goToColumn moves the cursor to the given column.
    void goToColumn(int column);

    // centerCursor centers the displayed cursor if possible
    // end centers the scroll on the cursor.
    void centerCursor();

    // goToOccurrence goes to the next occurence of string if any, previous one
    // if backward is set.
    void goToOccurrence(const QString& string, bool backward);

    void up();
    void down();
    void left();
    void right();

    int currentLineNumber();
    int currentColumn();

    // saveCheckpoint stores the current filename/cursor position information has a checkpoint.
    void saveCheckpoint();

    // lastCheckpoint pops the last checkpoint and goes into this file / position.
    void lastCheckpoint();

    // text manipulation
    // -----------------

    // deleteCurrentLine removes the current line of the buffer.
    void deleteCurrentLine();

    // insertNewLine insert a new line either under, or above the current one
    // and takes care of adding the proper indentation.
    // If noCutText is set to true, while creating/going to the next line,
    // the text on the // right of the cursor won't be moved to the new line.
    // When above is true, noCutText has no effect.
    void insertNewLine(bool above, bool noCutText = false);

    // getWordUnderCursor returns the word under the cursor if any.
    QString getWordUnderCursor();

    // removeIndentation removes one level of indentation on the line of the given cursor..
    void removeIndentation(QTextCursor cursor);

    // insertIndentation adds one level of indentation on the line of the given cursor.
    void insertIndentation(QTextCursor cursor);

    // highlightText highlights the given text in the editor.
    void highlightText(QString text);

    // open state of buffers
    // ---------------------

    // storeOpenedState stores whether the given file is opened or not.
    bool storeOpenedState(const QString& filepath);

    // removeOpenedState deletes the "open" flag for this file.
    void removeOpenedState(const QString& filepath);

    // LSP
    // -------------

    // XXX(remy):
    // autocomplete is a basic auto-complete with words available in the opened
    // buffers. It is trying to complete the current word if one has been started.
    void autocomplete();

    // lspAutocomplete is using an available LSP client if any for the current buffer
    // for smart auto-complete.
    void lspAutocomplete();

    void applyAutocomplete(const QString& base, const QString& word);

    // showLSPDiagnosticsOfLine shows the diagnostics for this line (of the current buffer)
    // in the statusbar.
    void showLSPDiagnosticsOfLine(int line);

    // lspInterpretMessages is called when data has been received from the LSP server
    // and that the editor needs to interpret them. Note that it can interpret several
    // messages in on call (if the LSP server has sent several messages in one output)
    void lspInterpretMessages(const QByteArray& data);

    // lspInterpret is called by the LSP server to let the Editor interpret
    // one JSON message.
    void lspInterpret(QJsonDocument json);

    LSPManager* lspManager;

    // widget related
    // --------------

    // getFont returns the font used by the editor.
    static QFont getFont();

    // called by the window when it is resized.
    void onWindowResized(QResizeEvent*);

    QTextBlock getFirstVisibleBlock() const { return this->firstVisibleBlock(); }

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();

    // lineNumberAtY returns which line number is at the Y value.
    int lineNumberAtY(int y);

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

    // cleanOnlyWhiteSpacesLine removes all the whitespace of the current line.
    void cleanOnlyWhiteSpacesLine();

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

    // alreadyOpened returns true if another editor instance seems to have
    // already opened this file.
    // It is using the settings file to do that.
    bool alreadyOpened(const QString& filepath);

    // selectVisualLineSelection selects blocks selected during the visual line
    // movement.
    void selectVisualLineSelection();

    // getStatusBar is a convenient method returns the Window's StatusBar instance.
    StatusBar* getStatusBar();

    // checkpoints stored.
    QList<Checkpoint> checkpoints;

    // ----------------------

    TasksPlugin *tasksPlugin;

    // ----------------------

    QTimer* selectionTimer;
    QTimer* lspRefreshTimer;
    QListWidget* currentCompleter;
    QWidget* lineNumberArea;

    Window* window;
    Syntax* syntax;

    // currentBuffer is the currently visible buffer. Note that it is not part
    // of the buffers map.
    Buffer* currentBuffer;

    // buffers is the currently loaded buffers. Note that it doesn't contain
    // the currentBuffer. It is the owner of the buffers.
    // The key is the ID of the buffer, could be a filename but it could also be
    // just a simple name for buffers not written on disk.
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

    // visualLineBlockStart is the block at which has been started the visual
    // line mode.
    QTextBlock visualLineBlockStart;
};
