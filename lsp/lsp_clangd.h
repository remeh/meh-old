#include <QList>
#include <QString>

#include "../buffer.h"

class CompleterEntry;
class LSP;
class LSPWriter;
class Window;

class LSPClangd : public LSP
{
public:
    LSPClangd(Window* window, const QString& baseDir);
    ~LSPClangd() override;
    void readStandardOutput() override;

    // protocol
    bool start() override;
    void openFile(Buffer* buffer) override;
    void refreshFile(Buffer* buffer) override;
    void initialize() override;
    void definition(int reqId, const QString& filename, int line, int column) override;
    void declaration(int reqId, const QString& filename, int line, int column) override;
    void hover(int reqId, const QString& filename, int line, int column) override;
    void signatureHelp(int reqId, const QString& filename, int line, int column) override;
    void references(int reqId, const QString& filename, int line, int column) override;
    void completion(int reqId, const QString& filename, int line, int column) override;
    QList<CompleterEntry> getEntries(const QJsonDocument& json);

private:
    QString baseDir;
    LSPWriter writer;
};
