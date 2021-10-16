#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "../completer.h"
#include "../lsp.h"
#include "../window.h"
#include "generic.h"

LSPGeneric::LSPGeneric(Window* window, const QString& baseDir, const QString& language, const QString& command, QStringList args) : LSP(window) {
    this->serverSpawned = false;
    this->language = language;
    this->baseDir = baseDir;
    this->command = command;
    this->args = args;
}

LSPGeneric::~LSPGeneric() {
    this->lspServer.kill();
    this->lspServer.waitForFinished(1000);
}

void LSPGeneric::readStandardOutput() {
    if (this->window == nullptr) {
        return;
    }
    QByteArray data = this->lspServer.readAll();
    this->window->lspInterpretMessages(data);
}

// --------------------------

bool LSPGeneric::start() {
    this->lspServer.start(this->command, this->args);
    this->serverSpawned = this->lspServer.waitForStarted(5000);
    return this->serverSpawned;
}

void LSPGeneric::initialize(Buffer* buffer) {
    Q_ASSERT(buffer != nullptr);

    const QString& initialize = this->writer.initialize(this->baseDir);
    const QString& initialized = this->writer.initialized();
    this->lspServer.write(initialize.toUtf8());
    this->window->getLSPManager()->setExecutedAction(1, LSP_ACTION_INIT, buffer);
    this->lspServer.write(initialized.toUtf8());
}

void LSPGeneric::openFile(Buffer* buffer) {
    Q_ASSERT(buffer != nullptr);

    const QString& msg = this->writer.openFile(buffer, buffer->getFilename(), this->language);
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::refreshFile(Buffer* buffer) {
    Q_ASSERT(buffer != nullptr);

    const QString& msg = this->writer.refreshFile(buffer, buffer->getFilename());
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::definition(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.definition(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::declaration(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.declaration(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::hover(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.hover(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::signatureHelp(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.signatureHelp(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::references(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.references(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

void LSPGeneric::completion(int reqId, const QString& filename, int line, int column) {
    const QString& msg = this->writer.completion(reqId, filename, line, column);
    this->lspServer.write(msg.toUtf8());
}

QList<CompleterEntry> LSPGeneric::getEntries(const QJsonDocument& json) {
    QList<CompleterEntry> list;

    if (json["result"].isNull() || json["result"]["items"].isNull()) {
        return list;
    }

    QJsonArray items = json["result"]["items"].toArray();

    if (items.size() == 0) {
        this->window->getStatusBar()->setMessage("Nothing found.");
        return list;
    }

    for (int i = 0; i < items.size(); i++) {
        QJsonObject object = items[i].toObject();
        list.append(CompleterEntry(object["label"].toString(), object["detail"].toString()));
    }

    return list;
}
