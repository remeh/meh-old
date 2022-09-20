#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "../completer.h"
#include "../lsp.h"
#include "../window.h"
#include "clangd.h"
#include "generic.h"

LSPClangd::LSPClangd(Window* window, const QString& baseDir) : LSP(window) {
    this->generic = new LSPGeneric(window, baseDir, "cpp", "clangd", QStringList() << "--completion-style=detailed");
}

LSPClangd::~LSPClangd() {
    delete this->generic;
}

void LSPClangd::readStandardOutput() {
    this->generic->readStandardOutput();
}

// --------------------------

bool LSPClangd::start() {
    return this->generic->start();
}

void LSPClangd::initialize(Buffer* buffer) {
    this->generic->initialize(buffer);
}

void LSPClangd::openFile(Buffer* buffer) {
    this->generic->openFile(buffer);
}

void LSPClangd::refreshFile(Buffer* buffer) {
    this->generic->refreshFile(buffer);
}

void LSPClangd::definition(int reqId, const QString& filename, int line, int column) {
    this->generic->definition(reqId, filename, line, column);
}

void LSPClangd::declaration(int reqId, const QString& filename, int line, int column) {
    this->generic->declaration(reqId, filename, line, column);
}

void LSPClangd::hover(int reqId, const QString& filename, int line, int column) {
    this->generic->hover(reqId, filename, line, column);
}

void LSPClangd::signatureHelp(int reqId, const QString& filename, int line, int column) {
    this->generic->signatureHelp(reqId, filename, line, column);
}

void LSPClangd::references(int reqId, const QString& filename, int line, int column) {
    this->generic->references(reqId, filename, line, column);
}

void LSPClangd::completion(int reqId, const QString& filename, int line, int column) {
    this->generic->completion(reqId, filename, line, column);
}

QString LSPClangd::getLanguage() {
    return this->generic->getLanguage();
}

QList<CompleterEntry> LSPClangd::getEntries(const QJsonDocument& json) {
    QList<CompleterEntry> list;

    if (json[tr("result")].isNull() || json[tr("result")][tr("items")].isNull()) {
        return list;
    }

    QJsonArray items = json[tr("result")][tr("items")].toArray();

    if (items.size() == 0) {
        this->window->getStatusBar()->setMessage("Nothing found.");
        return list;
    }


    for (int i = 0; i < items.size(); i++) {
        QJsonObject object = items[i].toObject();
        list.append(CompleterEntry(object[tr("insertText")].toString(), object[tr("label")].toString(), LSPReader::isFunc(object[tr("kind")].toInteger(13))));
    }

    return list;
}
