#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "../completer.h"
#include "../lsp.h"
#include "../window.h"
#include "gopls.h"

#include "qdebug.h"

LSPGopls::LSPGopls(Window* window, const QString& baseDir) : LSP(window) {
    this->serverSpawned = false;
    this->language = "go";
    this->baseDir = baseDir;
}

LSPGopls::~LSPGopls() {
    this->lspServer.kill();
    this->lspServer.waitForFinished(1000);
}

void LSPGopls::readStandardOutput() {
    if (this->window == nullptr) {
        return;
    }
    QByteArray data = this->lspServer.readAll();
    this->window->getEditor()->lspInterpretMessages(data);
}

// --------------------------

bool LSPGopls::start() {
    this->lspServer.start("gopls", QStringList());
    this->serverSpawned = this->lspServer.waitForStarted(5000);
    if (!this->serverSpawned) {
        this->window->getStatusBar()->setMessage("Unable to start gopls binary");
    }
    return this->serverSpawned;
}

void LSPGopls::initialize() {
    const QString& initialize = this->writer.initialize(this->baseDir);
    const QString& initialized = this->writer.initialized();
    this->window->getEditor()->lspManager.setExecutedAction(this->window, 1, LSP_ACTION_INIT, this->window->getEditor()->getCurrentBuffer());
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

void LSPGopls::hover(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.hover(reqId, filename, line, column);
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

QList<CompleterEntry> LSPGopls::getEntries(const QJsonDocument& json) {
    QList<CompleterEntry> list;

    if (json["result"].isNull() || json["result"]["items"].isNull()) {
        return list;
    }

    QJsonArray items = json["result"]["items"].toArray();

    if (items.size() == 0) {
        this->window->getStatusBar()->setMessage("Nothing found.");
        return list;
    }

    QList<CompleterEntry> entries;
    for (int i = 0; i < items.size(); i++) {
        QJsonObject object = items[i].toObject();
        list.append(CompleterEntry(object["label"].toString(), object["detail"].toString()));
    }

    return list;
}
