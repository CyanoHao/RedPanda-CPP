cmake_minimum_required(VERSION 3.19)
project(Red_Panda_CPP
    LANGUAGES C CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(QT5 "Build with Qt 5" OFF)
option(LUA_ADDON "Enable Lua add-ons" ON)
option(SDCC "Enable SDCC compiler support" ON)
option(VCS "Enable Git integration" OFF)
option(WINDOWS_PREFER_OPENCONSOLE "Prefer OpenConsole.exe as terminal emulator" OFF)

set(LIBEXECDIR libexec CACHE STRING "Directory for auxiliary executables, RELATIVE TO prefix.")
set(APP_NAME RedPandaCPP CACHE STRING "Subdirectory name to $prefix/share and $prefix/libexec.")

if (QT5)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Network PrintSupport Svg Widgets Xml LinguistTools)
else()
    find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Network PrintSupport Svg Widgets Xml LinguistTools)
endif()

# read version from "version.json"
file(READ version.json VERSION_JSON)
string(JSON APP_VERSION_MAJOR GET ${VERSION_JSON} major)
string(JSON APP_VERSION_MINOR GET ${VERSION_JSON} minor)
string(JSON APP_VERSION_PATCH GET ${VERSION_JSON} patch)
string(JSON APP_VERSION_PRE_RELEASE GET ${VERSION_JSON} preRelease)

# try git commit count for version sorting
execute_process(
    COMMAND git rev-list HEAD --count
    OUTPUT_VARIABLE APP_VERSION_BUILD
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if (APP_VERSION_BUILD)
    set(REDPANDA_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}.${APP_VERSION_BUILD}")
else()
    set(REDPANDA_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")
endif()

if (APP_VERSION_PRE_RELEASE)
    set(REDPANDA_VERSION "${REDPANDA_VERSION}-${APP_VERSION_PRE_RELEASE}")
endif()

if (UNIX AND NOT APPLE)
    set(XDG ON)
else()
    set(XDG OFF)
endif()

function(target_moc_classes target)
    foreach(arg IN LISTS ARGN)
        target_sources(${target} PRIVATE ${arg}.cpp ${arg}.h)
    endforeach()
endfunction()

function(target_ui_classes target)
    foreach(arg IN LISTS ARGN)
        target_sources(${target} PRIVATE ${arg}.cpp ${arg}.h ${arg}.ui)
    endforeach()
endfunction()

function(target_embed_translations target)
    set(embed_translation_qm_dir ${CMAKE_CURRENT_BINARY_DIR}/.qm)
    set(embed_translation_qrc_dir ${CMAKE_CURRENT_BINARY_DIR}/.qrc)
    file(MAKE_DIRECTORY ${embed_translation_qm_dir})
    file(MAKE_DIRECTORY ${embed_translation_qrc_dir})

    set(embed_translation_qrc_file ${embed_translation_qm_dir}/${target}_qmake_qmake_qm_files.qrc)
    set(embed_translation_qm_files)
    set_source_files_properties(${ARGN} PROPERTIES OUTPUT_LOCATION ${embed_translation_qm_dir})
    qt_add_translation(embed_translation_qm_files ${ARGN})

    file(WRITE ${embed_translation_qrc_file}
        "<RCC>"
            "<qresource prefix=\"/i18n\">"
    )

    foreach(qm_file IN LISTS embed_translation_qm_files)
        file(RELATIVE_PATH qm_filename ${embed_translation_qm_dir} ${qm_file})
        file(APPEND ${embed_translation_qrc_file}
                "<file alias=\"${qm_filename}\">${qm_file}</file>"
        )
    endforeach()

    file(APPEND ${embed_translation_qrc_file}
            "</qresource>"
        "</RCC>"
    )

    target_sources(${target} PRIVATE ${embed_translation_qrc_file})
endfunction()

function(target_embed_resources target resource_name)
    set(embed_resource_qrc_dir ${CMAKE_CURRENT_BINARY_DIR}/.qrc)
    file(MAKE_DIRECTORY ${embed_resource_qrc_dir})

    set(embed_resource_qrc_file ${embed_resource_qrc_dir}/${resource_name}.qrc)

    file(WRITE ${embed_resource_qrc_file}
        "<RCC>"
            "<qresource prefix=\"/\">"
    )

    foreach(resource_file IN LISTS ARGN)
        file(RELATIVE_PATH resource_filename ${CMAKE_CURRENT_SOURCE_DIR} ${resource_file})
        file(APPEND ${embed_resource_qrc_file}
                "<file alias=\"${resource_filename}\">${resource_file}</file>"
        )
    endforeach()

    file(APPEND ${embed_resource_qrc_file}
            "</qresource>"
        "</RCC>"
    )

    target_sources(${target} PRIVATE ${embed_resource_qrc_file})
endfunction()

add_subdirectory(RedPandaIDE)
if (LUA_ADDON)
    add_subdirectory(libs/lua)
endif()
add_subdirectory(libs/qsynedit)
add_subdirectory(libs/redpanda_qt_utils)
add_subdirectory(tools/consolepauser)

if (VCS)
    if (WIN32)
        add_subdirectory(tools/redpanda-win-git-askpass)
    else()
        add_subdirectory(tools/redpanda-git-askpass)
    endif()
endif()

if (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0501)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

if (MSVC)
    add_compile_definitions(NOMINMAX)
endif()

#############
# Resources #
#############

# docs
if (XDG)
    install(
        FILES README.md NEWS.md LICENSE
        DESTINATION share/doc/${APP_NAME}
    )
else()
    install(
        FILES README.md NEWS.md LICENSE
        DESTINATION .
    )
endif()

# icon
if (XDG)
    install(
        FILES platform/linux/redpandaide.svg
        DESTINATION share/icons/hicolor/scalable/apps
    )
endif()

# desktop entry
IF(XDG)
    configure_file(
        platform/linux/RedPandaIDE.desktop.in
        RedPandaIDE.desktop
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/RedPandaIDE.desktop
        DESTINATION share/applications
    )
endif()

# mime type
if(XDG)
    install(
        FILES platform/linux/redpandaide.xml
        DESTINATION share/mime/packages
    )
endif()

# qt.conf
if(WIN32)
    install(
        FILES platform/windows/qt.conf
        DESTINATION .
    )
endif()
