#pragma once

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTcpSocket>

#include "buffer.h"

class LSP;
class Window;

// LSPWriter is used to generate the LSP messages.
class LSPWriter
{
public:
    QString initialize(const QString& baseDir);
    QString initialized();
    QString openFile(Buffer* buffer, const QString& filepath, const QString& language);
    QString definition(const QString& filename, int line, int column);
    QString declaration(const QString& filename, int line, int column);
    QString signatureHelp(const QString& filename, int line, int column);
    QString references(const QString& filename, int line, int column);

protected:
private:
    QString payload(QString& content);
};

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

    bool manageBuffer(Window* window, Buffer* buffer);
    LSP* getLSP(Buffer* buffer);

protected:
private:

    // forLanguage returns the LSP instance available for the given
    // language. If nullptr is returned, no instance exists for this language.
    LSP* forLanguage(const QString& language);
    // filename -> lsp
    QMap<QString, LSP*> lspsPerFile;
    // list of instanciated lsps.
    QList<LSP*> lsps;
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
    virtual void initialize() = 0;
    virtual void definition(const QString& filename, int line, int column) = 0;
    virtual void declaration(const QString& filename, int line, int column) = 0;
    virtual void signatureHelp(const QString& filename, int line, int column) = 0;
    virtual void references(const QString& filename, int line, int column) = 0;

    const QString& getLanguage() { return this->language; }
private slots:
    void readyReadStandardOutput();
protected:
    QProcess lspServer;
    bool serverSpawned;
    QString language;
private:
};
