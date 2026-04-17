cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(qtermwidget)

include(GNUInstallDirs)
include(GenerateExportHeader)

option(BUILD_QTERMWIDGET_EXAMPLES "Build example application" OFF)

set(QTERMWIDGET_VERSION_MAJOR "2")
set(QTERMWIDGET_VERSION_MINOR "3")
set(QTERMWIDGET_VERSION_PATCH "0")
set(QTERMWIDGET_VERSION "${QTERMWIDGET_VERSION_MAJOR}.${QTERMWIDGET_VERSION_MINOR}.${QTERMWIDGET_VERSION_PATCH}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6Widgets REQUIRED)
find_package(Qt6LinguistTools REQUIRED)

set(QTERMWIDGET_LIBRARY_NAME qtermwidget6)

set(SRCS
    lib/BlockArray.cpp
    lib/ColorScheme.cpp
    lib/Emulation.cpp
    lib/Filter.cpp
    lib/History.cpp
    lib/HistorySearch.cpp
    lib/KeyboardTranslator.cpp
    lib/konsole_wcwidth.cpp
    lib/kprocess.cpp
    lib/kpty.cpp
    lib/kptydevice.cpp
    lib/kptyprocess.cpp
    lib/Pty.cpp
    lib/qtermwidget.cpp
    lib/Screen.cpp
    lib/ScreenWindow.cpp
    lib/SearchBar.cpp
    lib/Session.cpp
    lib/ShellCommand.cpp
    lib/TerminalCharacterDecoder.cpp
    lib/TerminalDisplay.cpp
    lib/tools.cpp
    lib/Vt102Emulation.cpp
)

set(HDRS
    lib/Emulation.h
    lib/Filter.h
    lib/HistorySearch.h
    lib/kprocess.h
    lib/kptydevice.h
    lib/kptyprocess.h
    lib/Pty.h
    lib/qtermwidget.h
    lib/ScreenWindow.h
    lib/SearchBar.h
    lib/Session.h
    lib/TerminalDisplay.h
    lib/Vt102Emulation.h
)

set(UI
    lib/SearchBar.ui
)

set(HDRS_DISTRIB
    lib/qtermwidget.h
    lib/Emulation.h
    lib/KeyboardTranslator.h
    lib/Filter.h
    lib/qtermwidget_interface.h
)

set(KB_LAYOUT_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/kb-layouts")
message(STATUS "Keyboard layouts will be installed in: ${KB_LAYOUT_DIR}")

set(COLORSCHEMES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/color-schemes")
message(STATUS "Color schemes will be installed in: ${COLORSCHEMES_DIR}")

set(TRANSLATIONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/translations")
message(STATUS "Translations will be installed in: ${TRANSLATIONS_DIR}")

qt6_wrap_cpp(MOCS ${HDRS})
qt6_wrap_ui(UI_SRCS ${UI})

# Build static library
add_library(${QTERMWIDGET_LIBRARY_NAME} STATIC ${SRCS} ${MOCS} ${UI_SRCS})
target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} PUBLIC Qt6::Widgets)
set_target_properties(${QTERMWIDGET_LIBRARY_NAME} PROPERTIES
    VERSION ${QTERMWIDGET_VERSION}
    SOVERSION ${QTERMWIDGET_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

if(APPLE)
    target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME} PRIVATE
        HAVE_UTMPX
        UTMPX_COMPAT
    )
endif()

target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME} PRIVATE
    KB_LAYOUT_DIR="${KB_LAYOUT_DIR}"
    COLORSCHEMES_DIR="${COLORSCHEMES_DIR}"
    TRANSLATIONS_DIR="${TRANSLATIONS_DIR}"
    HAVE_POSIX_OPENPT
    HAVE_SYS_TIME_H
)

generate_export_header(${QTERMWIDGET_LIBRARY_NAME}
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_export.h"
    BASE_NAME QTERMWIDGET
)

target_include_directories(${QTERMWIDGET_LIBRARY_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/lib>"
    INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}>"
)

# Generate version header
configure_file(
    "${PROJECT_SOURCE_DIR}/lib/qtermwidget_version.h.in"
    "${CMAKE_BINARY_DIR}/lib/qtermwidget_version.h"
    @ONLY
)

# Install headers
install(FILES
    ${HDRS_DISTRIB}
    "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_export.h"
    "${CMAKE_BINARY_DIR}/lib/qtermwidget_version.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

# Install library
install(TARGETS ${QTERMWIDGET_LIBRARY_NAME}
    EXPORT ${QTERMWIDGET_LIBRARY_NAME}-targets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Install keyboard layouts
install(DIRECTORY lib/kb-layouts/
    DESTINATION "${KB_LAYOUT_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.keytab"
)

# Install color schemes
install(DIRECTORY lib/color-schemes/
    DESTINATION "${COLORSCHEMES_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.*schem*"
)

# Install translations
install(DIRECTORY lib/translations/
    DESTINATION "${TRANSLATIONS_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.qm"
)

# Install translation sources
install(FILES lib/translations/qtermwidget.ts
    DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/${QTERMWIDGET_LIBRARY_NAME}/translation-sources"
    COMPONENT Devel
)

# Example application
if(BUILD_QTERMWIDGET_EXAMPLES)
    add_executable(example examples/cpp/main.cpp)
    target_link_libraries(example ${QTERMWIDGET_LIBRARY_NAME})
endif()

# Export targets
install(EXPORT ${QTERMWIDGET_LIBRARY_NAME}-targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
    NAMESPACE ${QTERMWIDGET_LIBRARY_NAME}::
    COMPONENT Devel
)

export(TARGETS ${QTERMWIDGET_LIBRARY_NAME}
    FILE "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-targets.cmake"
)
