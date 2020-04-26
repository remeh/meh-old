QT += widgets

HEADERS     =   buffer.h \
                command.h \
                editor.h \
                fileslookup.h \
                grep.h \
                syntax.h \
                window.h
SOURCES     =   buffer.cpp \
                command.cpp \
                editor.cpp \
                fileslookup.cpp \
                grep.cpp \
                main.cpp \
                normal.cpp \
                syntax.cpp \
                visual.cpp \
                window.cpp

# CONFIG += sanitizer sanitize_address
