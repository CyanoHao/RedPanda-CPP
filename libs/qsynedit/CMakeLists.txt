qt_add_library(qsynedit
    qsynedit/codefolding.cpp qsynedit/codefolding.h
    qsynedit/constants.cpp qsynedit/constants.h
    qsynedit/document.cpp qsynedit/document.h
    qsynedit/exporter/exporter.cpp qsynedit/exporter/exporter.h
    qsynedit/exporter/htmlexporter.cpp qsynedit/exporter/htmlexporter.h
    qsynedit/exporter/qtsupportedhtmlexporter.cpp qsynedit/exporter/qtsupportedhtmlexporter.h
    qsynedit/exporter/rtfexporter.cpp qsynedit/exporter/rtfexporter.h
    qsynedit/formatter/cppformatter.cpp qsynedit/formatter/cppformatter.h
    qsynedit/formatter/formatter.cpp qsynedit/formatter/formatter.h
    qsynedit/gutter.cpp qsynedit/gutter.h
    qsynedit/keystrokes.cpp qsynedit/keystrokes.h
    qsynedit/miscprocs.cpp qsynedit/miscprocs.h
    qsynedit/painter.cpp qsynedit/painter.h
    qsynedit/qsynedit.cpp qsynedit/qsynedit.h
    qsynedit/searcher/baseseacher.cpp qsynedit/searcher/baseseacher.h
    qsynedit/searcher/basicsearcher.cpp qsynedit/searcher/basicsearcher.h
    qsynedit/searcher/regexsearcher.cpp qsynedit/searcher/regexsearcher.h
    qsynedit/syntaxer/asm.cpp qsynedit/syntaxer/asm.h
    qsynedit/syntaxer/cpp.cpp qsynedit/syntaxer/cpp.h
    qsynedit/syntaxer/glsl.cpp qsynedit/syntaxer/glsl.h
    qsynedit/syntaxer/lua.cpp qsynedit/syntaxer/lua.h
    qsynedit/syntaxer/makefile.cpp qsynedit/syntaxer/makefile.h
    qsynedit/syntaxer/syntaxer.cpp qsynedit/syntaxer/syntaxer.h
    qsynedit/syntaxer/textfile.cpp qsynedit/syntaxer/textfile.h
    qsynedit/types.cpp qsynedit/types.h
)
target_include_directories(qsynedit PUBLIC
    ../redpanda_qt_utils
)

target_link_libraries(qsynedit PUBLIC
    Qt::Core
    Qt::Gui
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(qsynedit PUBLIC
        Qt::Widgets
    )
endif()

if((QMAKE_HOST.arch___contains___x86_64) AND (QMAKE_HOST.arch___contains___x86_64))
    target_compile_definitions(qsynedit PUBLIC
        ARCH_X86_64=1
    )
endif()

if((QMAKE_HOST.arch___contains___x86_64) AND (NOT (QMAKE_HOST.arch___contains___x86_64)))
    target_compile_definitions(qsynedit PUBLIC
        ARCH_X86=1
    )
endif()

if(WIN32)
    target_compile_definitions(qsynedit PUBLIC
        _WIN32_WINNT=0x0501
    )
endif()

if(MSVC)
    target_compile_definitions(qsynedit PUBLIC
        NOMINMAX
    )
endif()

install(TARGETS qsynedit
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
