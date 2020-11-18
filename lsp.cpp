#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringList>
#include <QTextStream>

#include "buffer.h"
#include "lsp.h"
#include "window.h"

#include "qdebug.h"

LSP::LSP(Window* window) : QObject(window) {
    this->window = window;
    connect(&this->lspServer, &QProcess::readyReadStandardOutput, this, &LSP::readyReadStandardOutput);
}

void LSP::readyReadStandardOutput() {
    this->readStandardOutput();
}

LSP::~LSP() {
}

// --------------------------

QString LSPWriter::initialize(const QString& baseDir) {
    QString content;
    QFileInfo fi(baseDir);
    QJsonParseError* error = nullptr;
    QJsonObject workspace;




    QJsonObject dynRegTrue { {"dynamicRegistration", true} };
    QJsonObject dynRegFalse { {"dynamicRegistration", false} };

    QJsonObject {
        {"snippetSupport", false},
        {"commitCharactersSupport", false},
        {"documentationFormat", QJsonArray { "plaintext" }},
        {"deprecatedSupport", false }
    };

    QJsonObject textDocument {
        {"completion", QJsonObject {
            {"dynamicRegistration", true},
            {"contextSupport", true},
            {"completionItem", QJsonObject {
                {"snippetSupport", false},
                {"commitCharactersSupport", false},
                {"documentationFormat", QJsonArray { "plaintext" }},
                {"deprecatedSupport", false }
            }}
        }},
        {"hover", QJsonObject {
            {"dynamicRegistration", true},
            {"contentFormat", QJsonArray { "plaintext "}}
        }},
        {"signatureHelp", QJsonObject {
            {"dynamicRegistration", true},
            {"signatureInformation", QJsonObject {
                { "documentationFormat", QJsonArray { "plaintext "}}
            }}
        }},
        {"publishDiagnostics", dynRegTrue},
        {"synchronization",    dynRegTrue},
        {"codeAction",         dynRegTrue},
        {"typeDefinition",     dynRegTrue},
        {"implementation",     dynRegTrue},
        {"definition",         dynRegTrue},
        {"references",         dynRegTrue},
        {"documentHighlight",  dynRegFalse},
        {"documentSymbol",     dynRegFalse},
        {"formatting",         dynRegFalse},
        {"rangeFormatting",    dynRegFalse},
        {"rename",             dynRegFalse},
        {"documentLink",       dynRegFalse}
    };
    QJsonObject capabilities {
        {"workspace", workspace},
        {"textDocument", textDocument}
    };
    QJsonObject params {
        {"rootUri", "file://" + fi.absoluteFilePath()},
        {"processId", QCoreApplication::applicationPid()},
        {"capabilities", capabilities},
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::initialized() {
    QJsonObject params;
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"method", "initialized"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::openFile(Buffer* buffer, const QString& filename, const QString& language) {
    QJsonObject textDocument {
        {"uri", "file://" + filename },
        {"version", 1},
        {"languageId", language},
        {"text", QString(buffer->getData())}
    };
    QJsonObject params {
        {"textDocument", textDocument}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::refreshFile(Buffer* buffer, const QString& filename) {
    QJsonObject textDocument {
        {"uri", "file://" + filename },
        {"version", QString::number(QDateTime::currentMSecsSinceEpoch()) },
    };
    QJsonObject contentChange {
        {"text", QString(buffer->getData())},
    };
    QJsonArray contentChanges {
        contentChange,
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"contentChanges", contentChanges},
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::definition(int reqId, const QString& filename, int line, int column) {
    QJsonObject position {
        {"line", line},
        {"character", column}
    };
    QJsonObject textDocument {
        {"uri", "file://" + filename },
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"position", position}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", "textDocument/definition"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::declaration(int reqId, const QString& filename, int line, int column) {
    QJsonObject position {
        {"line", line},
        {"character", column}
    };
    QJsonObject textDocument {
        {"uri", "file://" + filename },
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"position", position}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", "textDocument/declaration"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::hover(int reqId, const QString& filename, int line, int column) {
    QJsonObject position {
        {"line", line},
        {"character", column}
    };
    QJsonObject textDocument {
        {"uri", "file://" + filename },
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"position", position}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", "textDocument/hover"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::signatureHelp(int reqId, const QString& filename, int line, int column) {
    QJsonObject position {
        {"line", line},
        {"character", column}
    };
    QJsonObject textDocument {
        {"uri", "file://" + filename },
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"position", position}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", "textDocument/signatureHelp"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::references(int reqId, const QString& filename, int line, int column) {
    QJsonObject position {
        {"line", line},
        {"character", column}
    };
    QJsonObject textDocument {
        {"uri", "file://" + filename },
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"position", position}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", "textDocument/references"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::completion(int reqId, const QString& filename, int line, int column) {
    QJsonObject position {
        {"line", line},
        {"character", column}
    };
    QJsonObject textDocument {
        {"uri", "file://" + filename },
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"position", position}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", "textDocument/completion"},
        {"params", params}
    };
    QString str = QString(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return this->payload(str);
}

QString LSPWriter::payload(QString& content) {
    int size = content.size();
    content.prepend("Content-Length: " + QString::number(size) + "\r\n\r\n");
    return content;
}

// --------------------------

QJsonDocument LSPReader::readMessage(const QByteArray& message) {
    QJsonDocument empty;

    QRegularExpression rx(QStringLiteral("Content-Length: (\\d+)"));
    QRegularExpressionMatchIterator it = rx.globalMatch(message);
    int payloadSize = 0;
    if (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        payloadSize = match.captured(1).toInt();
    } else {
        qWarning() << "LSPReader::readMessage" << "can't read the received message from the LSP Server. Message:";
        qWarning() << message;
        return empty;
    }

    QByteArray payload = message.right(payloadSize);
    QJsonParseError* error = nullptr;
    QJsonDocument result = QJsonDocument::fromJson(payload, error);
    if (error != nullptr) {
        qWarning() << "LSPReader::readMessage" << "can't unmarshal the payload:" << error;
        return empty;
    }

    return result;
}
