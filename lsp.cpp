#include <QCoreApplication>
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
    QJsonObject params;
    QJsonObject workspace = QJsonDocument::fromJson(" \
        { \
            \"applyEdit\": true, \
            \"workspaceEdit\": { \
                \"documentChanges\": true \
            }, \
            \"didChangeConfiguration\": { \
                \"dynamicRegistration\": true \
            }, \
            \"didChangeWatchedFiles\": { \
                \"dynamicRegistration\": true \
            }, \
            \"symbol\": { \
                \"dynamicRegistration\": true, \
                \"symbolKind\": { \
                    \"valueSet\": [ \
                        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, \
                        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 \
                    ] \
                } \
            }, \
            \"executeCommand\": { \
                \"dynamicRegistration\": true \
            }, \
            \"configuration\": true, \
            \"workspaceFolders\": true \
        } \
    ").object();
    QJsonObject textDocument = QJsonDocument::fromJson(" \
        { \
            \"publishDiagnostics\": { \
                \"relatedInformation\": true \
            }, \
            \"synchronization\": { \
                \"dynamicRegistration\": true, \
                \"willSave\": true, \
                \"willSaveWaitUntil\": true, \
                \"didSave\": true \
            }, \
            \"completion\": { \
                \"dynamicRegistration\": true, \
                \"contextSupport\": true, \
                \"completionItem\": { \
                    \"snippetSupport\": true, \
                    \"commitCharactersSupport\": true, \
                    \"documentationFormat\": [ \
                        \"markdown\", \
                        \"plaintext\" \
                    ], \
                    \"deprecatedSupport\": true \
                }, \
                \"completionItemKind\": { \
                    \"valueSet\": [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, \
                        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 ] \
                } \
            }, \
            \"hover\": { \
                \"dynamicRegistration\": true, \
                \"contentFormat\": [ \"markdown\", \"plaintext\" ] \
            }, \
            \"signatureHelp\": { \
                \"dynamicRegistration\": true, \
                \"signatureInformation\": { \
                    \"documentationFormat\": [ \"markdown\", \"plaintext\" ] \
                } \
            }, \
            \"definition\": { \"dynamicRegistration\": true }, \
            \"references\": { \"dynamicRegistration\": true }, \
            \"documentHighlight\": { \"dynamicRegistration\": true }, \
            \"documentSymbol\": { \
                \"dynamicRegistration\": true, \
                \"symbolKind\": { \
                    \"valueSet\": [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, \
                        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 ] \
                } \
            }, \
            \"codeAction\": { \"dynamicRegistration\": true }, \
            \"codeLens\": { \"dynamicRegistration\": true }, \
            \"formatting\": { \"dynamicRegistration\": true }, \
            \"rangeFormatting\": { \"dynamicRegistration\": true }, \
            \"onTypeFormatting\": { \"dynamicRegistration\": true }, \
            \"rename\": { \"dynamicRegistration\": true }, \
            \"documentLink\": { \"dynamicRegistration\": true }, \
            \"typeDefinition\": { \"dynamicRegistration\": true }, \
            \"implementation\": { \"dynamicRegistration\": true }, \
            \"colorProvider\": { \"dynamicRegistration\": true }, \
            \"foldingRange\": { \
                \"dynamicRegistration\": false, \
                \"rangeLimit\": 5000, \
                \"lineFoldingOnly\": true \
            } \
        } \
    ").object();
    QJsonObject capabilities {
        {"workspace", workspace},
        {"textDocument", textDocument}
    };
    QJsonObject object {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"rootPath", fi.absoluteFilePath()},
        {"rootUri", "file://" + fi.absoluteFilePath()},
        {"processId", QCoreApplication::applicationPid()},
        {"capabilities", capabilities},
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
        // NOTE(remy): is file version mandatory?
    };
    QJsonObject contentChange {
        {"text", QString(buffer->getData())},
    };
    QJsonArray contentChanges {
        contentChange,
    };
    QJsonObject params {
        {"textDocument", textDocument},
        {"contentChanges", contentChanges },
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
        qWarning() << "LSPReader::readMessage" << "can't read the received message from the LSP Server.";
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
