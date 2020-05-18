QT += widgets network

HEADERS     =   buffer.h \
                command.h \
                completer.h \
                editor.h \
                fileslookup.h \
                grep.h \
                lsp.h \
                syntax.h \
                window.h
SOURCES     =   buffer.cpp \
                command.cpp \
                completer.cpp \
                editor.cpp \
                fileslookup.cpp \
                grep.cpp \
                lsp.cpp \
                main.cpp \
                normal.cpp \
                submode.cpp \
                syntax.cpp \
                visual.cpp \
                window.cpp

# CONFIG += sanitizer sanitize_address
# Debug symbols
# QMAKE_CXXFLAGS += -g
# Remove optimizations
# QMAKE_CXXFLAGS_RELEASE -= -O2
