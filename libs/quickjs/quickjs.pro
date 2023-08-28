TEMPLATE = lib

CONFIG += staticlib

DEFINES += \
    _GNU_SOURCE \
    CONFIG_VERSION=\\\"2021-03-27\\\" \
    CONFIG_BIGNUM

win32 {
    DEFINES += __USE_MINGW_ANSI_STDIO
}

gcc {
    QMAKE_CFLAGS = -Wall -Wno-array-bounds -Wno-format-truncation
}

clang {
    QMAKE_CFLAGS = -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers -Wundef -Wuninitialized -Wunused -Wno-unused-parameter -Wwrite-strings -Wchar-subscripts -funsigned-char
}

SOURCES += \
    quickjs/quickjs.c \
    quickjs/libregexp.c \
    quickjs/libunicode.c \
    quickjs/cutils.c \
    quickjs/quickjs-libc.c \
    quickjs/libbf.c

HEADERS += \
    quickjs/quickjs.h
