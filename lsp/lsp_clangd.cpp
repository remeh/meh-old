#include <QDir>
#include <QFileInfo>

#include "lsp.h"
#include "lsp/lsp_clangd.h"

#include "qdebug.h"

LSPClangd::LSPClangd(const QString& baseDir) {
    this->serverSpawned = false;
    this->language = "cpp";
    this->baseDir = baseDir;
}

LSPClangd::~LSPClangd() {
    this->lspServer.terminate();
    this->lspServer.waitForFinished(5000);
}

bool LSPClangd::start() {
    this->lspServer.start("clangd");
    this->serverSpawned = this->lspServer.waitForStarted(5000);
    qDebug() << "LSPClangd started";
    // TODO(remy): send the initialize command
    return this->serverSpawned;
}

void LSPClangd::initialize() {
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

void LSPClangd::openFile(Buffer* buffer) {
    const QString& msg = this->writer.openFile(buffer, buffer->getFilename(), this->language);
    qDebug() << "sending:" << msg;
    int written = this->lspServer.write(msg.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}

void LSPClangd::definition(const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(filename, line, column);
    qDebug() << "sending\n" << msg;
    int written = this->lspServer.write(msg.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}
