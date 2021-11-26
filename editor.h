#pragma once

#include <QChar>
#include <QColor>
#include <QFocusEvent>
#include <QFont>
#include <QIcon>
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
#include "syntax_highlighter.h"
#include "tasks.h"

class Window;
class LineNumberArea;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    Editor(Window* window);
    ~Editor();

    Window* getWindow() { return this->window; }

    // buffer
    // ------

    // the buffer this editor has been created for
    Buffer* buffer;
    Buffer* getBuffer() { return this->buffer; }
    void setBuffer(Buffer* buffer);

    // saves the currently opened buffer
    void save();

    QString getId() {
        Q_ASSERT(this->buffer != nullptr);
        return this->buffer->getId();
    }

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

    // at the start of the line, move onto the first char instead of being in
    // the indentation
    void moveToFirstWord(QTextCursor* cursor);

    int currentLineNumber();
    int currentColumn();

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

    // setSearchText sets the pattern to highlight because
    // of a search
    void setSearchText(QString text);

    // getOneLine returns the given line in the given file.
    // It returns the last line of the file if line doesn't exist.
    const QString getOneLine(const QString filename, int line);

    // open state of buffers
    // ---------------------

    // storeOpenedState stores whether the given file is opened or not.
    bool storeOpenedState(const QString& filepath);

    // removeOpenedState deletes the "open" flag for this file.
    void removeOpenedState(const QString& filepath);

    // LSP
    // -------------

    // autocomplete is a basic auto-complete with words available in the opened
    // buffers. It is trying to complete the current word if one has been started.
    void autocomplete();

    // lspAutocomplete is using an available LSP client if any for the current buffer
    // for smart auto-complete.
    void lspAutocomplete();

    void applyAutocomplete(const QString& type, const QString& base, const QString& word, const QString& popup);

    // widget related
    // --------------

    // getFont returns the font used by the editor.
    static QFont getFont();

    QIcon getIcon();

    // called by the window when it is resized.
    void onWindowResized(QResizeEvent*);

    QTextBlock getFirstVisibleBlock() const { return this->firstVisibleBlock(); }

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();

    // lineNumberAtY returns which line number is at the Y value.
    int lineNumberAtY(int y);

    // tab index in the window

    void setTabIndex(int idx) { this->tabIndex = idx; }
    int getTabIndex() { return this->tabIndex; }
    LineNumberArea* lineNumberArea;

public slots:
    void update();

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
    // ignoreLineReturn can be set in order to get the "last char before the line return".
    QChar currentLineLastChar(bool ignoreLineReturn = false);

    // findNextOneInCurrentLine returns the distance to the next occurence of the
    // given char in the current line. The distance is from the current cursor position.
    int findNextOneInCurrentLine(QChar c);

    // findPreviousOneInCurrentLine returns the distance to the previous occurence of
    // the given char in the current line. The distance is from the current cursor position.
    int findPreviousOneInCurrentLine(QChar c);

    // bufferExtension returns the extension of the current file.
    // Returns an empty string if the buffer has no extension.
    QString bufferExtension();

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

    // ----------------------

    TasksPlugin *tasksPlugin;

    // ----------------------

    QTimer* selectionTimer;
    QTimer* lspRefreshTimer;
    QListWidget* currentCompleter;

    Window* window;
    SyntaxHighlighter* syntax;

    // mode is the currently used mode. See mode.h
    int mode;

    // subMode is the currently used mode. See mode.h
    int subMode;

    // tabIndex is the position of the editor in the list of tab
    int tabIndex;

    // eightCharsX is the X position where the eighty chars line must be drawn.
    int eightyCharsX;
    int hundredTwentyCharsX;

    QColor highlightedLine;

    // visualLineBlockStart is the block at which has been started the visual
    // line mode.
    QTextBlock visualLineBlockStart;
};
