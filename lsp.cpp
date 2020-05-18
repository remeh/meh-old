#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

#include "buffer.h"
#include "lsp.h"
#include "window.h"

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
        qDebug() << "Starting LSPGo";
        LSP* lsp = new LSPGo(window->getBaseDir());
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
    for (int i = 0; i < this->lsps.size(); i++) {
        if (this->lsps.at(i)->getLanguage() == language) {
            return this->lsps.at(i);
        }
    }
    return nullptr;
}

bool LSPManager::manageBuffer(Window* window, Buffer* buffer) {
    Q_ASSERT(window != nullptr);
    Q_ASSERT(buffer != nullptr);

    // already managed
    if (this->lspsPerFile.contains(buffer->getFilename())) {
        return true;
    }

    QFileInfo fi(buffer->getFilename());
    LSP* lsp = this->forLanguage(fi.suffix());
    if (lsp == nullptr) {
        qDebug() << "Has to start an LSP";
        lsp = this->start(window, fi.suffix());
    }
    if (lsp != nullptr) {
        qDebug() << "for" << buffer->getFilename() << "use lsp" << lsp;
        this->lspsPerFile.insert(buffer->getFilename(), lsp);
        // TODO(remy): sends the message to the LSP server to open this file
        lsp->openFile(buffer->getFilename());
        return true;
    }

    qDebug() << "No LSP available";
    return false;
}

LSP* LSPManager::getLSP(Buffer* buffer) {
    Q_ASSERT(buffer != nullptr);

    if (!this->lspsPerFile.contains(buffer->getFilename())) {
        return nullptr;
    }

    return this->lspsPerFile.value(buffer->getFilename(), nullptr);
}

LSP::~LSP() {
}

LSPGo::LSPGo(const QString& baseDir) {
    this->serverSpawned = false;
    this->language = "go";
    this->baseDir = baseDir;
}

LSPGo::~LSPGo() {
    this->lspServer.terminate();
    this->lspServer.waitForFinished(5000);
}

bool LSPGo::start() {
    this->lspServer.start("gopls");
    this->serverSpawned = this->lspServer.waitForStarted(5000);
    qDebug() << "LSPGo started";
    // TODO(remy): send the initialize command
    return this->serverSpawned;
}

void LSPGo::initialize() {
    const QString& initialize = this->writer.initialize(this->baseDir);
    const QString& initialized = this->writer.initialized();
    qDebug() << "sending:" << initialize;
    int written = this->lspServer.write(initialize.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
    qDebug() << "sending:" << initialized;
    written = this->lspServer.write(initialized.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}

void LSPGo::openFile(const QString& filename) {
    // gopls wants directories and not files
    QFileInfo fi(filename);
    qDebug() << "directory:" << fi.absoluteDir().absolutePath();
    const QString& msg = this->writer.openFile(fi.absoluteDir().absolutePath(), "go");
    qDebug() << "sending:" << msg;
    int written = this->lspServer.write(msg.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}

void LSPGo::definition(const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(filename, line, column);
    qDebug() << "sending\n" << msg;
    int written = this->lspServer.write(msg.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}

QString LSPWriter::initialize(const QString& baseDir) {
    QString content;
    QFileInfo fi(baseDir);
    QTextStream(&content) << "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{"
                          << "\"rootPath\":\"" << fi.absoluteFilePath() << "\","
                          << "\"rootUri\":\"" << fi.absoluteFilePath() << "\","
                          << "\"processId\":" << QCoreApplication::applicationPid() << ","
                          << "\"capabilities\":{"
                          <<   "\"workspace\":"
                          <<     "\"definition\":{\"dynamicRegistration\":true},"
                          <<     "\"references\":{\"dynamicRegistration\":true},"
                          <<     "\"implementation\":{\"dynamicRegistration\":true}"
                          <<   "}"
                          << "}}";
    return this->payload(content);
}

QString LSPWriter::initialized() {
    QString content = "{\"jsonrpc\":\"2.0\",\"method\":\"initialized\",\"params\":{}}";
    return this->payload(content);
}

QString LSPWriter::openFile(const QString& filename, const QString& language) {
    QString content;
    QTextStream(&content) << "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"textDocument/didOpen\",\"params\":{\"textDocument\":{\"uri\":\"file://"
                          << filename << "\",\"language\":\"" << language << "\"}}}";
    // TODO(remy): should we append the whole file?
    return this->payload(content);
}

QString LSPWriter::definition(const QString& filename, int line, int column) {
    QString content;
    QTextStream(&content) << "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"textDocument/definition\",\"params\":{\"textDocument\":{\"uri\":\"file://"
                          << filename << "\"},\"position\":{\"line\":"
                          << line << ",\"character\":"
                          << column << "}}}";
    return this->payload(content);
}

QString LSPWriter::payload(QString& content) {
    int size = content.size();
    content.prepend("Content-Length: " + QString::number(size) + "\r\n\r\n");
    return content;
}
