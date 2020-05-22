QT += widgets network

HEADERS     =   buffer.h \
                command.h \
                completer.h \
                editor.h \
                fileslookup.h \
                grep.h \
                lsp.h \
                lsp/lsp_clangd.h \
                lsp/lsp_gopls.h \
                syntax.h \
                window.h
SOURCES     =   buffer.cpp \
                command.cpp \
                completer.cpp \
                editor.cpp \
                fileslookup.cpp \
                grep.cpp \
                lsp.cpp \
                lsp/lsp_clangd.cpp \
                lsp/lsp_gopls.cpp \
                main.cpp \
                normal.cpp \
                submode.cpp \
                syntax.cpp \
                visual.cpp \
                window.cpp

RESOURCES   =   resources.qrc

# CONFIG += sanitizer sanitize_address
# Debug symbols
# QMAKE_CXXFLAGS += -g
# Remove optimizations
# QMAKE_CXXFLAGS_RELEASE -= -O2
