#include <QDir>
#include <QFileInfo>

#include "../lsp.h"
#include "lsp_gopls.h"

#include "qdebug.h"

LSPGopls::LSPGopls(Window* window, const QString& baseDir) : LSP(window) {
    this->serverSpawned = false;
    this->language = "go";
    this->baseDir = baseDir;
}

LSPGopls::~LSPGopls() {
    this->lspServer.terminate();
    this->lspServer.waitForFinished(5000);
}

void LSPGopls::readStandardOutput() {
    qDebug() << "received";
    qDebug() << this->lspServer.readAll();
}

// --------------------------

bool LSPGopls::start() {
    this->lspServer.start("gopls");
    this->serverSpawned = this->lspServer.waitForStarted(5000);
    qDebug() << "LSPGopls started";
    // TODO(remy): send the initialize command
    return this->serverSpawned;
}

void LSPGopls::initialize() {
    const QString& initialize = this->writer.initialize(this->baseDir);
    const QString& initialized = this->writer.initialized();
    this->lspServer.write(initialize.toUtf8());
    this->lspServer.write(initialized.toUtf8());
}

void LSPGopls::openFile(Buffer* buffer) {
    // gopls wants directories and not files
    QFileInfo fi(buffer->getFilename());
    const QString& msg = this->writer.openFile(buffer, fi.absoluteDir().absolutePath(), "go");
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::definition(const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::declaration(const QString& filename, int line, int column) {
    const QString& msg = this->writer.declaration(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::signatureHelp(const QString& filename, int line, int column) {
    const QString& msg = this->writer.signatureHelp(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::references(const QString& filename, int line, int column) {
    const QString& msg = this->writer.references(filename, line, column);
    this->lspServer.write(msg.toUtf8());
}
