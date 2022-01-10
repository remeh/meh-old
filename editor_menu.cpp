#include <QAction>
#include <QList>
#include <QMenu>
#include <QPoint>
#include <QTextBlock>

#include "editor.h"
#include "window.h"

#include "qdebug.h"

void Editor::contextMenuEvent(QContextMenuEvent* event) {
    LSP* lsp = this->window->getLSPManager()->getLSP(this->buffer->getId());

    // we store it to access position slightly later
    this->menuOpenedEvent = event;

    QMenu menu;

    QAction* information = menu.addAction(tr("Information"), this, &Editor::onMenuInfo);
    QAction* definition = menu.addAction(tr("Go to definition"), this, &Editor::onMenuGoToDef);
    QAction* references = menu.addAction(tr("References"), this, &Editor::onMenuReferences);

    if (lsp == nullptr) {
        information->setDisabled(true);
        definition->setDisabled(true);
        references->setDisabled(true);
    }

    menu.addSeparator();

    QAction* copy = menu.addAction(tr("Copy"), this, &Editor::onMenuCopy);
    QAction* cut = menu.addAction(tr("Cut"), this, &Editor::onMenuCut);
    QAction* paste = menu.addAction(tr("Paste"), this, &Editor::onMenuPaste);

    QTextCursor cursor = this->textCursor();
    QString text = cursor.selectedText();
    if (text.size() <= 0) {
        copy->setDisabled(true);
        cut->setDisabled(true);
    }

    menu.addSeparator();

    menu.addAction(tr("Rg"), this, &Editor::onMenuRg);

    menu.exec(event->globalPos());
}

void Editor::menuGetLineAndColumn(int* line, int* column) {
    Q_ASSERT(line != nullptr);
    Q_ASSERT(column != nullptr);

    if (this->menuOpenedEvent == nullptr) {
        *line = 0;
        *column = 0;
        return;
    }

    QPoint pos = this->menuOpenedEvent->pos();
    pos.setX(this->lineNumberAreaWidth() + pos.x() - this->viewportMargins().left());
    pos.setY(pos.y() - this->viewportMargins().top());
    QTextCursor cursor = this->cursorForPosition(pos);

    *line = cursor.blockNumber() + 1;
    *column = cursor.columnNumber();
}

void Editor::onMenuLspCall(int lspAction) {
    LSP* lsp = this->window->getLSPManager()->getLSP(this->buffer->getId());
    if (lsp == nullptr) {
        return;
    }

    int line, column;
    this->menuGetLineAndColumn(&line, &column);

    int reqId = this->window->getLSPManager()->randomId();
    switch (lspAction) {
        case LSP_ACTION_HOVER:
        case LSP_ACTION_HOVER_MOUSE:
            lsp->hover(reqId, buffer->getFilename(), line, column);
            break;
        case LSP_ACTION_REFERENCES:
            lsp->references(reqId, buffer->getFilename(), line, column);
            break;
        case LSP_ACTION_DEFINITION:
            lsp->definition(reqId, buffer->getFilename(), line, column);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
    this->window->getLSPManager()->setExecutedAction(reqId, lspAction, this->buffer);
}

void Editor::onMenuReferences() {
    this->onMenuLspCall(LSP_ACTION_REFERENCES);
}

void Editor::onMenuInfo() {
    this->onMenuLspCall(LSP_ACTION_HOVER_MOUSE);
}

void Editor::onMenuGoToDef() {
    this->onMenuLspCall(LSP_ACTION_DEFINITION);
}

void Editor::onMenuCopy() {
    this->copy();
}
void Editor::onMenuCut() {
    this->cut();
}
void Editor::onMenuPaste() {
    this->paste();
}
void Editor::onMenuRg() {
    QTextCursor cursor = this->textCursor();
    QString text = cursor.selectedText();

    if (text.size() == 0) {
        text = this->window->getEditor()->getWordUnderCursor();
    }

    if (text.size() == 0) {
        return;
    }

    this->window->openGrep(text);
}
