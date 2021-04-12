#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTime>

#include "buffer.h"

// consider a request to the LSP server timeouted after 5 seconds
// if no reply has been received
#define LSP_ACTION_TIMEOUT_S 5

class LSP;
class Window;

typedef struct LSPAction {
    int requestId;
    int action;
    Buffer* buffer;
    QTime creationTime;
} LSPAction;

typedef struct LSPDiagnostic {
    QString absFilename;
    QString message;
    int line;
} LSPDiagnostic;

class LSPManager : public QObject
{
    Q_OBJECT
public:
    LSPManager(Window* window);
    ~LSPManager();

    // start initializes the server and initializes a client for
    // the given language. Synchronous calls returning a nullptr
    // if an error occurred. This pointer returned must not be
    // freed.
    LSP* start(const QString& language);

    // manageBuffer let the LSP server managers the given buffer.
    // If it is already managed, it will refresh it in the LSP cache.
    bool manageBuffer(Buffer* buffer);

    // reload deletes all existing lsp instances and restart one for
    // the given buffer
    void reload(Buffer* buffer);

    // getLSP returns the LSP server managing the given buffer.
    LSP* getLSP(Buffer* buffer);

    // setExecutedAction stores which action has been executed for the given
    // request ID. The buffer the user was into at this moment is also stored.
    void setExecutedAction(int reqId, int action, Buffer* buffer);

    // getExecutedAction returns information on the executed action for the
    // given request ID.
    LSPAction getExecutedAction(int reqId);

    // TODO(remy): comment me
    void addDiagnostic(const QString& absFilename, LSPDiagnostic diag);

    // TODO(remy): comment me
    QMap<int, QList<LSPDiagnostic>> getDiagnostics(const QString& absFilename);

    // TODO(remy): comment me
    void clearDiagnostics(const QString& absFilename);

protected:
private:
    // cleanTimer is used to regularly clean action triggered where there is
    // more than XXsecond.
    QTimer* cleanTimer;

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

    Window* window;

private slots:
    // timeoutActions looks whether or not there is actions to delete because
    // we considered them timeouted.
    void timeoutActions();
};
