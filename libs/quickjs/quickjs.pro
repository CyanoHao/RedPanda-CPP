TEMPLATE = lib

CONFIG += staticlib

SOURCES += \
    quickjs/cutils.c \
    quickjs/libbf.c \
    quickjs/libregexp.c \
    quickjs/libunicode.c \
    quickjs/quickjs.c

HEADERS += \
    quickjs/quickjs.h
