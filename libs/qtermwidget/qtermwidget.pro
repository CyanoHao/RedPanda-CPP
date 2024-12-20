TEMPLATE = lib
QT += core gui widgets

CONFIG += c++17
CONFIG += nokey
CONFIG += staticlib
CONFIG += lrelease
CONFIG += embed_translations
QMAKE_RESOURCE_FLAGS += -name $(QMAKE_TARGET)_${QMAKE_FILE_BASE}

DEFINES += \
    KB_LAYOUT_DIR=\\\":/qtermwidget/kb-layouts\\\" \
    COLORSCHEMES_DIR=\\\":/qtermwidget/color-schemes\\\" \
    TRANSLATIONS_DIR=\\\":/qtermwidget/translations\\\" \
    HAVE_POSIX_OPENPT \
    HAVE_SYS_TIME_H

SOURCES += \
    qtermwidget/BlockArray.cpp \
    qtermwidget/ColorScheme.cpp \
    qtermwidget/Emulation.cpp \
    qtermwidget/Filter.cpp \
    qtermwidget/History.cpp \
    qtermwidget/HistorySearch.cpp \
    qtermwidget/KeyboardTranslator.cpp \
    qtermwidget/konsole_wcwidth.cpp \
    qtermwidget/kprocess.cpp \
    qtermwidget/kpty.cpp \
    qtermwidget/kptydevice.cpp \
    qtermwidget/kptyprocess.cpp \
    qtermwidget/Pty.cpp \
    qtermwidget/qtermwidget.cpp \
    qtermwidget/Screen.cpp \
    qtermwidget/ScreenWindow.cpp \
    qtermwidget/SearchBar.cpp \
    qtermwidget/Session.cpp \
    qtermwidget/ShellCommand.cpp \
    qtermwidget/TerminalCharacterDecoder.cpp \
    qtermwidget/TerminalDisplay.cpp \
    qtermwidget/tools.cpp \
    qtermwidget/Vt102Emulation.cpp

HEADERS += \
    qtermwidget/Emulation.h \
    qtermwidget/Filter.h \
    qtermwidget/HistorySearch.h \
    qtermwidget/kprocess.h \
    qtermwidget/kptydevice.h \
    qtermwidget/kptyprocess.h \
    qtermwidget/Pty.h \
    qtermwidget/qtermwidget.h \
    qtermwidget/ScreenWindow.h \
    qtermwidget/SearchBar.h \
    qtermwidget/Session.h \
    qtermwidget/TerminalDisplay.h \
    qtermwidget/Vt102Emulation.h

FORMS += \
    qtermwidget/SearchBar.ui

INCLUDEPATH += qtermwidget
