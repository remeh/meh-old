#include <QDir>
#include <QFileInfo>

#include "../lsp.h"
#include "../window.h"
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
    if (this->window == nullptr) {
        return;
    }
    QByteArray data = this->lspServer.readAll();
    this->window->getEditor()->lspInterpret(data);
}

// --------------------------

bool LSPGopls::start() {
    this->lspServer.start("gopls");
    this->serverSpawned = this->lspServer.waitForStarted(5000);
    return this->serverSpawned;
}

void LSPGopls::initialize() {
    const QString& initialize = this->writer.initialize(this->baseDir);
    const QString& initialized = this->writer.initialized();
    this->lspServer.write(initialize.toUtf8());
    this->lspServer.write(initialized.toUtf8());
}

void LSPGopls::openFile(Buffer* buffer) {
    QFileInfo fi(buffer->getFilename());
    const QString& msg = this->writer.openFile(buffer, fi.absoluteDir().absolutePath(), "go");
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::refreshFile(Buffer* buffer) {
    const QString& msg = this->writer.refreshFile(buffer, buffer->getFilename());
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::definition(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::declaration(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.declaration(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::signatureHelp(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.signatureHelp(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::references(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.references(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGopls::completion(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.completion(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}
