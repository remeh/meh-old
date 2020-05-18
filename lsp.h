#pragma once

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTcpSocket>

#include "buffer.h"

class LSP;
class LSPGo;
class Window;

// LSPWriter is used to generate the LSP messages.
class LSPWriter
{
public:
    QString initialize(const QString& baseDir);
    QString initialized();
    QString openFile(const QString& filename, const QString& language);
    QString definition(const QString& filename, int line, int column);

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

class LSP
{
public:
    virtual ~LSP();
    virtual bool start() = 0;
    virtual void openFile(const QString& filename) = 0;
    virtual void initialize() = 0;
    virtual void definition(const QString& filename, int line, int column) = 0;

    const QString& getLanguage() { return this->language; }

protected:
    QProcess lspServer;
    bool serverSpawned;
    QString language;
private:
};

class LSPGo : public LSP
{
public:
    LSPGo(const QString& baseDir);
    ~LSPGo() override;
    bool start() override;
    void openFile(const QString& filename) override;
    void initialize() override;
    void definition(const QString& filename, int line, int column);
private:
    QString baseDir;
    LSPWriter writer;
};
