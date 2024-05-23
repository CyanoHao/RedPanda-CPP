TEMPLATE = lib

CONFIG += staticlib

unix {
    macos {
        DEFINES += LUA_USE_MACOSX
    } else {
        DEFINES += LUA_USE_LINUX  # BSDs are Linux in Lua source
    }
}

SOURCES += \
    compact_enc_det/compact_enc_det.cc \
    compact_enc_det/compact_enc_det_hint_code.cc \
    util/encodings/encodings.cc \
    util/languages/languages.cc

HEADERS += \
    compact_enc_det/compact_enc_det.h \
    compact_enc_det/compact_enc_det_hint_code.h
