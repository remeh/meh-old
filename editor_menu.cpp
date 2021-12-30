#include <QAction>
#include <QList>
#include <QMenu>
#include <QPoint>
#include <QTextBlock>

#include "editor.h"
#include "window.h"

void Editor::contextMenuEvent(QContextMenuEvent* event) {
    LSP* lsp = this->window->getLSPManager()->getLSP(this->buffer->getId());
    if (lsp == nullptr) {
        return;
    }

    // we store it to access position slightly later
    this->menuOpenedEvent = event;

    // TODO(remy): show only if LSP's not null

    QMenu menu;

    QAction* information = menu.addAction(tr("Information"), this, &Editor::onMenuInfo);
    QAction* definition = menu.addAction(tr("Go to definition"), this, &Editor::onMenuGoToDef);

    if (lsp != nullptr) {
        information->setDisabled(true);
        definition->setDisabled(true);
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


void Editor::onMenuInfo() {
    LSP* lsp = this->window->getLSPManager()->getLSP(this->buffer->getId());
    if (lsp == nullptr) {
        return;
    }

    int line, column;
    this->menuGetLineAndColumn(&line, &column);

    int reqId = this->window->getLSPManager()->randomId();
    lsp->hover(reqId, buffer->getFilename(), line, column);
    this->window->getLSPManager()->setExecutedAction(reqId, LSP_ACTION_HOVER_MOUSE, this->buffer);
}

void Editor::onMenuGoToDef() {
    LSP* lsp = this->window->getLSPManager()->getLSP(this->buffer->getId());
    if (lsp == nullptr) {
        return;
    }

    int line, column;
    this->menuGetLineAndColumn(&line, &column);

    int reqId = this->window->getLSPManager()->randomId();
    lsp->definition(reqId, buffer->getFilename(), line, column);
    this->window->getLSPManager()->setExecutedAction(reqId, LSP_ACTION_DEFINITION, this->buffer);
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
