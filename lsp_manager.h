#pragma once

#include <QList>
#include <QMap>
#include <QString>

#include "buffer.h"

class LSP;
class Window;

typedef struct LSPAction {
    int requestId;
    int action;
    Buffer* buffer;
} LSPAction;

typedef struct LSPDiagnostic {
    QString absFilename;
    QString message;
    int line;
} LSPDiagnostic;

class LSPManager
{
public:
    LSPManager();
    ~LSPManager();

    // start initializes the server and initializes a client for
    // the given language. Synchronous calls returning a nullptr
    // if an error occurred. This pointer returned must not be
    // freed.
    LSP* start(Window* window, const QString& language);

    // manageBuffer let the LSP server managers the given buffer.
    // If it is already managed, it will refresh it in the LSP cache.
    bool manageBuffer(Window* window, Buffer* buffer);

    // getLSP returns the LSP server managing the given buffer.
    LSP* getLSP(Buffer* buffer);

    // setExecutedAction stores which action has been executed for the given
    // request ID. The buffer the user was into at this moment is also stored.
    void setExecutedAction(Window* window, int reqId, int action, Buffer* buffer);

    // getExecutedAction returns information on the executed action for the
    // given request ID.
    LSPAction getExecutedAction(Window* window, int reqId);

    // TODO(remy): comment me
    void addDiagnostic(const QString& absFilename, LSPDiagnostic diag);

    // TODO(remy): comment me
    QMap<int, QList<LSPDiagnostic>> getDiagnostics(const QString& absFilename);

    // TODO(remy): comment me
    void clearDiagnostics(const QString& absFilename);

protected:
private:

    // forLanguage returns the LSP instance available for the given
    // language. If nullptr is returned, no instance exists for this language.
    LSP* forLanguage(const QString& language);
    // filename -> lsp
    QMap<QString, LSP*> lspsPerFile;
    // list of instanciated lsps.
    QList<LSP*> lsps;
    // executedActions is the list of executed actions waiting for a response
    // from the LSP server.
    QMap<int, LSPAction> executedActions;

    // diagnostics is storing the diagnostics for the different files.
    QMap<QString, QMap<int, QList<LSPDiagnostic>>> diagnostics;
};
