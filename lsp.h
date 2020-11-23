#pragma once

#include <QMap>
#include <QObject>
#include <QJsonDocument>
#include <QProcess>
#include <QString>

#include "buffer.h"

#define LSP_ACTION_UNKNOWN 0
#define LSP_ACTION_DEFINITION 1
#define LSP_ACTION_DECLARATION 2
#define LSP_ACTION_SIGNATURE_HELP 3
#define LSP_ACTION_REFERENCES 4
#define LSP_ACTION_COMPLETION 5
#define LSP_ACTION_HOVER 6
#define LSP_ACTION_INIT 7

class CompleterEntry;
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
    QString hover(int reqId, const QString& filename, int line, int column);
    QString signatureHelp(int reqId, const QString& filename, int line, int column);
    QString references(int reqId, const QString& filename, int line, int column);
    QString completion(int reqId, const QString& filename, int line, int column);

protected:
private:
    QString payload(QString& content);
};

// LSPReader reads LSP messages.
class LSPReader
{
public:
    static QList<QJsonDocument> readMessage(QByteArray message);
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
    virtual void hover(int reqId, const QString& filename, int line, int column) = 0;
    virtual void signatureHelp(int reqId, const QString& filename, int line, int column) = 0;
    virtual void references(int reqId, const QString& filename, int line, int column) = 0;
    virtual void completion(int reqId, const QString& filename, int line, int column) = 0;
    virtual QList<CompleterEntry> getEntries(const QJsonDocument& json) = 0;

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
