#include <QString>

class LSP;
class LSPWriter;

class LSPClangd : public LSP
{
public:
    LSPClangd(const QString& baseDir);
    ~LSPClangd() override;
    bool start() override;
    void openFile(const QString& filename) override;
    void initialize() override;
    void definition(const QString& filename, int line, int column) override;
private:
    QString baseDir;
    LSPWriter writer;
};
