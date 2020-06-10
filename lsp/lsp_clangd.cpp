#include <QDir>
#include <QFileInfo>

#include "../lsp.h"
#include "lsp_clangd.h"

#include "qdebug.h"

LSPClangd::LSPClangd(Window* window, const QString& baseDir) : LSP(window) {
    this->serverSpawned = false;
    this->language = "cpp";
    this->baseDir = baseDir;
}

LSPClangd::~LSPClangd() {
    this->lspServer.terminate();
    this->lspServer.waitForFinished(5000);
}

void LSPClangd::readStandardOutput() {
    qDebug() << "received";
    qDebug() << this->lspServer.readAll();
}

// --------------------------

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
    this->lspServer.write(initialize.toUtf8());
    this->lspServer.write(initialized.toUtf8());
}

void LSPClangd::openFile(Buffer* buffer) {
    const QString& msg = this->writer.openFile(buffer, buffer->getFilename(), this->language);
    this->lspServer.write(msg.toUtf8());
}

void LSPClangd::definition(const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPClangd::declaration(const QString& filename, int line, int column) {
    const QString& msg = this->writer.declaration(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPClangd::signatureHelp(const QString& filename, int line, int column) {
    const QString& msg = this->writer.signatureHelp(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPClangd::references(const QString& filename, int line, int column) {
    const QString& msg = this->writer.references(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}