#include <QList>
#include <QString>
#include <QStringList>

#include "../buffer.h"

class CompleterEntry;
class LSP;
class LSPWriter;
class Window;

class LSPGeneric : public LSP
{
public:
    LSPGeneric(Window* window, const QString& baseDir, const QString& language, const QString& command, QStringList args);
    ~LSPGeneric() override;
    void readStandardOutput() override;

    // protocol
    bool start() override;
    void openFile(Buffer* buffer) override;
    void refreshFile(Buffer* buffer) override;
    void initialize(Buffer* buffer) override;
    void definition(int reqId, const QString& filename, int line, int column) override;
    void declaration(int reqId, const QString& filename, int line, int column) override;
    void hover(int reqId, const QString& filename, int line, int column) override;
    void signatureHelp(int reqId, const QString& filename, int line, int column) override;
    void references(int reqId, const QString& filename, int line, int column) override;
    void completion(int reqId, const QString& filename, int line, int column) override;
    QList<CompleterEntry> getEntries(const QJsonDocument& json) override;
    QString getLanguage() override { return this->language; };

private:
    QString baseDir;
    QString command;
    QString language;
    QStringList args;
    LSPWriter writer;
};
