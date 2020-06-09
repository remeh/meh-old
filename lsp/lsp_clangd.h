#include <QString>

#include "buffer.h"

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
    void initialize() override;
    void definition(const QString& filename, int line, int column) override;
private:
    QString baseDir;
    LSPWriter writer;
};
