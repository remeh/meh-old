#include <QList>
#include <QString>
#include <QTime>

#include "buffer.h"
#include "lsp.h"
#include "statusbar.h"
#include "window.h"
#include "lsp/clangd.h"
#include "lsp/gopls.h"
#include "lsp/zls.h"

#include "qdebug.h"

LSPManager::LSPManager(Window* window) : window(window) {
    this->cleanTimer = new QTimer;
    this->cleanTimer->start(2000); // run every 2s
    connect(this->cleanTimer, &QTimer::timeout, this, &LSPManager::timeoutActions);
}

LSPManager::~LSPManager() {
    this->cleanTimer->stop();
    delete this->cleanTimer;
    for (int i = 0; i < lsps.size(); i++) {
        LSP* lsp = lsps.takeAt(i);
        delete lsp;
    }
}

void LSPManager::timeoutActions() {
    QList<int> keys = this->executedActions.keys();
    QList<int> toRemove;
    for (int i = 0; i < keys.size(); i++) {
        LSPAction action = this->executedActions[keys.at(i)];
        // timeout
        if (action.creationTime.addSecs(LSP_ACTION_TIMEOUT_S) < QTime::currentTime()) {
            toRemove.append(keys.at(i));
        }
    }

    for (int i = 0; i < toRemove.size(); i++) {
        this->executedActions.remove(toRemove.at(i));
        qDebug() << "LSPManager::timeoutActions: timeout of request" << toRemove.at(i);
    }

    if (this->executedActions.size() == 0) {
        if (this->window != nullptr) {
            this->window->getStatusBar()->setLspRunning(false);
        }
    }

    this->cleanTimer->stop(); // run every 2s
    this->cleanTimer->start(2000); // run every 2s
}

LSP* LSPManager::start(const QString& language) {
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
    } else if (language == "zig") {
        LSP* lsp = new LSPZLS(window, window->getBaseDir());
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
    QString l = language;
    if (l == "h") { l = "cpp"; }
    for (int i = 0; i < this->lsps.size(); i++) {
        if (this->lsps.at(i)->getLanguage() == l) {
            return this->lsps.at(i);
        }
    }
    return nullptr;
}

bool LSPManager::manageBuffer(Buffer* buffer) {
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
        lsp = this->start(fi.suffix());
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

void LSPManager::reload(Buffer *buffer) {
    Q_ASSERT(buffer != nullptr);

    if (this->cleanTimer != nullptr) {
        this->cleanTimer->stop();
    }

    this->lspsPerFile.clear();
    for (int i = 0; i < lsps.size(); i++) {
        LSP* lsp = lsps.takeAt(i);
        delete lsp;
    }

    this->manageBuffer(buffer);
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
    a.creationTime = QTime::currentTime();
    this->executedActions.insert(reqId, a);
    if (window != nullptr) {
        window->getStatusBar()->setLspRunning(true);
    }
}

LSPAction LSPManager::getExecutedAction(int reqId) {
    if (!this->executedActions.contains(reqId)) {
        LSPAction action;
        action.requestId = 0;
        action.buffer = nullptr;
        action.action = LSP_ACTION_UNKNOWN;
        return action;
    }
    LSPAction action = this->executedActions.take(reqId);
    if (this->executedActions.size() == 0) {
        window->getStatusBar()->setLspRunning(false);
    }
    return action;
}

// Diagnostics
// -----------

void LSPManager::addDiagnostic(const QString& absFilepath, LSPDiagnostic diag) {
    QMap<int, QList<LSPDiagnostic>>& lists = this->diagnostics[absFilepath];
    QList<LSPDiagnostic>& list = lists[diag.line];
    list.append(diag);
    this->diagnostics[absFilepath].insert(diag.line, list);
}

QMap<int, QList<LSPDiagnostic>> LSPManager::getDiagnostics(const QString& absFilepath) {
    return this->diagnostics[absFilepath];
}

void LSPManager::clearDiagnostics(const QString& absFilepath) {
    this->diagnostics[absFilepath].clear();
    this->diagnostics.remove(absFilepath);
}
