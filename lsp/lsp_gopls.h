#include <QString>

#include "../buffer.h"

class LSP;
class LSPWriter;

class LSPGopls : public LSP
{
public:
    LSPGopls(const QString& baseDir);
    ~LSPGopls() override;
    bool start() override;
    void openFile(Buffer* buffer) override;
    void initialize() override;
    void definition(const QString& filename, int line, int column) override;
private:
    QString baseDir;
    LSPWriter writer;
};
