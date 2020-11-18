#include <QString>

#include "buffer.h"
#include "lsp.h"
#include "window.h"
#include "lsp/clangd.h"
#include "lsp/gopls.h"

#include "qdebug.h"

LSPManager::LSPManager() {
}

LSPManager::~LSPManager() {
    for (int i = 0; i < lsps.size(); i++) {
        LSP* lsp = lsps.takeAt(i);
        delete lsp;
    }
}

LSP* LSPManager::start(Window* window, const QString& language) {
    if (language == "go") {
         LSP* lsp = new LSPGopls(window, window->getBaseDir());
         if (!lsp->start()) {
             // TODO(remy): warning here
             delete lsp;
             return nullptr;
         }
         lsp->initialize();
         this->lsps.append(lsp);
         return lsp;
    } else if (language == "cpp" || language == "h") {
        LSP* lsp = new LSPClangd(window, window->getBaseDir());
        if (!lsp->start()) {
            // TODO(remy): warning here
            delete lsp;
            return nullptr;
        }
        lsp->initialize();
        this->lsps.append(lsp);
        return lsp;
    }
    // TODO(remy): warning here
    return nullptr;
}

LSP* LSPManager::forLanguage(const QString& language) {
    // XXX(remy):
    QString l = language;
    if (l == "h") { l = "cpp"; }
    for (int i = 0; i < this->lsps.size(); i++) {
        if (this->lsps.at(i)->getLanguage() == l) {
            return this->lsps.at(i);
        }
    }
    return nullptr;
}

bool LSPManager::manageBuffer(Window* window, Buffer* buffer) {
    Q_ASSERT(window != nullptr);
    Q_ASSERT(buffer != nullptr);

    // already managed
    bool refresh = false;
    if (this->lspsPerFile.contains(buffer->getFilename())) {
        refresh = true;
    }

    QFileInfo fi(buffer->getFilename());
    LSP* lsp = this->forLanguage(fi.suffix());
    if (lsp == nullptr) {
        lsp = this->start(window, fi.suffix());
    }
    if (lsp != nullptr) {
        if (refresh) {
            lsp->refreshFile(buffer);
        } else {
            this->lspsPerFile.insert(buffer->getFilename(), lsp);
            // TODO(remy): sends the message to the LSP server to open this file
            lsp->openFile(buffer);
        }
        return true;
    }
    return false;
}

LSP* LSPManager::getLSP(Buffer* buffer) {
    Q_ASSERT(buffer != nullptr);

    if (!this->lspsPerFile.contains(buffer->getFilename())) {
        return nullptr;
    }

    return this->lspsPerFile.value(buffer->getFilename(), nullptr);
}

void LSPManager::setExecutedAction(int reqId, int action, Buffer* buffer) {
    LSPAction a;
    a.requestId = reqId;
    a.action = action;
    a.buffer = buffer;
    this->executedActions.insert(reqId, a);
}

LSPAction LSPManager::getExecutedAction(int reqId) {
    if (!this->executedActions.contains(reqId)) {
        LSPAction action;
        action.requestId = 0;
        action.buffer = nullptr;
        action.action = LSP_ACTION_UNKNOWN;
        return action;
    }
    return this->executedActions.value(reqId);
}
