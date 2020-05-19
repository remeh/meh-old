#include <QDir>
#include <QFileInfo>

#include "lsp.h"
#include "lsp/lsp_gopls.h"

#include "qdebug.h"

LSPGopls::LSPGopls(const QString& baseDir) {
    this->serverSpawned = false;
    this->language = "go";
    this->baseDir = baseDir;
}

LSPGopls::~LSPGopls() {
    this->lspServer.terminate();
    this->lspServer.waitForFinished(5000);
}

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
    qDebug() << "sending:" << initialize;
    int written = this->lspServer.write(initialize.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
    qDebug() << "sending:" << initialized;
    written = this->lspServer.write(initialized.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}

void LSPGopls::openFile(Buffer* buffer) {
    // gopls wants directories and not files
    QFileInfo fi(buffer->getFilename());
    qDebug() << "directory:" << fi.absoluteDir().absolutePath();
    const QString& msg = this->writer.openFile(buffer, fi.absoluteDir().absolutePath(), "go");
    qDebug() << "sending:" << msg;
    int written = this->lspServer.write(msg.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}

void LSPGopls::definition(const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(filename, line, column);
    qDebug() << "sending\n" << msg;
    int written = this->lspServer.write(msg.toUtf8());
    qDebug() << written << "bytes written";
    qDebug() << this->lspServer.readAll();
}
