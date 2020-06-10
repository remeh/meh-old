#include <QString>

#include "../buffer.h"

class LSP;
class LSPWriter;
class Window;

class LSPGopls : public LSP
{
public:
    LSPGopls(Window* window, const QString& baseDir);
    ~LSPGopls() override;
    void readStandardOutput() override;

    // protocol
    bool start() override;
    void openFile(Buffer* buffer) override;
    void initialize() override;
    void definition(const QString& filename, int line, int column) override;
    void declaration(const QString& filename, int line, int column) override;
    void signatureHelp(const QString& filename, int line, int column) override;
    void references(const QString& filename, int line, int column) override;

private:
    QString baseDir;
    LSPWriter writer;
};
