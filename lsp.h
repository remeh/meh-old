#pragma once

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>

#include "buffer.h"

#define LSP_ACTION_UNKNOWN 0
#define LSP_ACTION_DEFINITION 1
#define LSP_ACTION_DECLARATION 2
#define LSP_ACTION_SIGNATURE_HELP 3
#define LSP_ACTION_REFERENCES 4
#define LSP_ACTION_COMPLETION 5

class LSP;
class Window;

// LSPWriter is used to generate the LSP messages.
class LSPWriter
{
public:
    QString initialize(const QString& baseDir);
    QString initialized();
    QString openFile(Buffer* buffer, const QString& filepath, const QString& language);
    QString refreshFile(Buffer* buffer, const QString& filepath);
    QString definition(int reqId, const QString& filename, int line, int column);
    QString declaration(int reqId, const QString& filename, int line, int column);
    QString signatureHelp(int reqId, const QString& filename, int line, int column);
    QString references(int reqId, const QString& filename, int line, int column);
    QString completion(int reqId, const QString& filename, int line, int column);

protected:
private:
    QString payload(QString& content);
};

typedef struct LSPAction {
    int requestId;
    int command;
    Buffer* buffer;
} LSPAction;

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

    // setExecutedAction stores which command has been executed for the given
    // request ID. The buffer the user was into at this moment is also stored.
    void setExecutedAction(int reqId, int command, Buffer* buffer);

    // getExecutedAction returns information on the executed action for the
    // given request ID.
    LSPAction getExecutedAction(int reqId);

    int removeExecutedAction(int reqId) { return this->executedActions.remove(reqId); };

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
};

class LSP : public QObject
{
    Q_OBJECT
public:
    LSP(Window* window);
    virtual ~LSP();

    // readStandardOutput is called when the LSP server has sent new data to read.
    virtual void readStandardOutput() = 0;

    // lsp protocol
    virtual bool start() = 0;
    virtual void openFile(Buffer* buffer) = 0;
    virtual void refreshFile(Buffer* buffer) = 0;
    virtual void initialize() = 0;
    virtual void definition(int reqId, const QString& filename, int line, int column) = 0;
    virtual void declaration(int reqId, const QString& filename, int line, int column) = 0;
    virtual void signatureHelp(int reqId, const QString& filename, int line, int column) = 0;
    virtual void references(int reqId, const QString& filename, int line, int column) = 0;
    virtual void completion(int reqId, const QString& filename, int line, int column) = 0;

    const QString& getLanguage() { return this->language; }
private slots:
    void readyReadStandardOutput();
protected:
    Window* window;
    QProcess lspServer;
    bool serverSpawned;
    QString language;
private:
};
