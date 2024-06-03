TEMPLATE = lib
CONFIG += staticlib c++17
QT += core

win32: {
    DEFINES += _WIN32_WINNT=0x0501
}

gcc {
    QMAKE_CXXFLAGS_RELEASE += -Werror=return-type
    QMAKE_CXXFLAGS_DEBUG += -Werror=return-type
}

SOURCES += \
    toolchain/buildmode.cpp \
    toolchain/toolchain.cpp

HEADERS += \
    toolchain/buildmode.h \
    toolchain/toolchain.h
