TARGET = editor

CONFIG += debug
CONFIG += c++11

CONFIG += qt
QT += widgets

SOURCES = \
    src/context.cpp \
    src/language.cpp \
    src/loader.cpp \
    src/main.cpp \
    src/rules.cpp \
    src/qutepart.cpp \
    src/syntax_highlighter.cpp \
    src/style.cpp \
    src/context_stack.cpp \


HEADERS = \
    src/context.h \
    src/language.h \
    src/qutepart.h \
    src/rules.h \
    src/syntax_highlighter.h \
    src/style.h \
    src/context_stack.h \
