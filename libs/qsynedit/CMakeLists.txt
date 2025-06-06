qt_add_library(qsynedit STATIC
    qsynedit/codefolding.cpp
    qsynedit/constants.cpp
    qsynedit/keystrokes.cpp
    qsynedit/miscprocs.cpp
    qsynedit/painter.cpp
    qsynedit/types.cpp
    # exporter
    qsynedit/exporter/exporter.cpp
    qsynedit/exporter/htmlexporter.cpp
    qsynedit/exporter/qtsupportedhtmlexporter.cpp
    qsynedit/exporter/rtfexporter.cpp
    # formatter
    qsynedit/formatter/cppformatter.cpp
    qsynedit/formatter/formatter.cpp
    # syntaxer
    qsynedit/syntaxer/asm.cpp
    qsynedit/syntaxer/cpp.cpp
    qsynedit/syntaxer/glsl.cpp
    qsynedit/syntaxer/lua.cpp
    qsynedit/syntaxer/makefile.cpp
    qsynedit/syntaxer/syntaxer.cpp
    qsynedit/syntaxer/textfile.cpp
)

target_moc_classes(qsynedit
    qsynedit/document
    qsynedit/gutter
    qsynedit/qsynedit
    # searcher
    qsynedit/searcher/baseseacher
    qsynedit/searcher/basicsearcher
    qsynedit/searcher/regexsearcher
)

target_embed_translations(qsynedit
    qsynedit_ru_RU.ts
    qsynedit_zh_CN.ts
)

target_include_directories(qsynedit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qsynedit PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    redpanda_qt_utils
)

target_compile_definitions(qsynedit PUBLIC
    ARCH_X86_64=1
    ARCH_X86=1
)
