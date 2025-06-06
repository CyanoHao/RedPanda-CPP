qt_add_executable(RedPandaIDE WIN32 MACOSX_BUNDLE
    SimpleIni.h
    addon/luaapi.h
    addon/luaexecutor.h
    addon/luaruntime.h
    autolinkmanager.cpp autolinkmanager.h
    caretlist.cpp caretlist.h
    codesnippetsmanager.cpp codesnippetsmanager.h
    colorscheme.cpp colorscheme.h
    compiler/compiler.cpp compiler/compiler.h
    compiler/compilerinfo.cpp compiler/compilerinfo.h
    compiler/compilermanager.cpp compiler/compilermanager.h
    compiler/executablerunner.cpp compiler/executablerunner.h
    compiler/filecompiler.cpp compiler/filecompiler.h
    compiler/ojproblemcasesrunner.cpp compiler/ojproblemcasesrunner.h
    compiler/projectcompiler.cpp compiler/projectcompiler.h
    compiler/runner.cpp compiler/runner.h
    compiler/stdincompiler.cpp compiler/stdincompiler.h
    customfileiconprovider.cpp
    debugger/debugger.cpp debugger/debugger.h
    debugger/gdbmidebugger.cpp debugger/gdbmidebugger.h
    debugger/gdbmiresultparser.cpp debugger/gdbmiresultparser.h
    mainwindow.ui
    settingsdialog/compilerautolinkwidget.ui
    settingsdialog/compilersetdirectorieswidget.ui
    settingsdialog/compilersetoptionwidget.ui
    settingsdialog/debuggeneralwidget.ui
    settingsdialog/editorautosavewidget.ui
    settingsdialog/editorclipboardwidget.ui
    settingsdialog/editorcodecompletionwidget.ui
    settingsdialog/editorcolorschemewidget.ui
    settingsdialog/editorcustomctypekeywords.ui
    settingsdialog/editorfontwidget.ui
    settingsdialog/editorgeneralwidget.ui
    settingsdialog/editormiscwidget.ui
    settingsdialog/editorsnippetwidget.ui
    settingsdialog/editorsymbolcompletionwidget.ui
    settingsdialog/editorsyntaxcheckwidget.ui
    settingsdialog/editortooltipswidget.ui
    settingsdialog/environmentappearancewidget.ui
    settingsdialog/environmentfolderswidget.ui
    settingsdialog/environmentperformancewidget.ui
    settingsdialog/environmentprogramswidget.ui
    settingsdialog/environmentshortcutwidget.ui
    settingsdialog/executorgeneralwidget.ui
    settingsdialog/executorproblemsetwidget.ui
    settingsdialog/formattergeneralwidget.ui
    settingsdialog/formatterpathwidget.ui
    settingsdialog/languageasmgenerationwidget.ui
    settingsdialog/projectcompileparamaterswidget.ui
    settingsdialog/projectcompilerwidget.ui
    settingsdialog/projectdirectorieswidget.ui
    settingsdialog/projectdllhostwidget.ui
    settingsdialog/projectfileswidget.ui
    settingsdialog/projectgeneralwidget.ui
    settingsdialog/projectmakefilewidget.ui
    settingsdialog/projectoutputwidget.ui
    settingsdialog/projectprecompilewidget.ui
    settingsdialog/settingsdialog.ui
    settingsdialog/toolsgeneralwidget.ui
    widgets/aboutdialog.ui
    widgets/choosethemedialog.ui
    widgets/cpudialog.ui
    widgets/custommakefileinfodialog.ui
    widgets/editorfontdialog.ui
    widgets/filepropertiesdialog.ui
    widgets/infomessagebox.ui
    widgets/newclassdialog.ui
    widgets/newheaderdialog.ui
    widgets/newprojectdialog.ui
    widgets/newprojectunitdialog.ui
    widgets/newtemplatedialog.ui
    widgets/ojproblempropertywidget.ui
    widgets/projectalreadyopendialog.ui
    widgets/searchdialog.ui
    widgets/searchinfiledialog.ui
    widgets/signalmessagedialog.ui
)
target_include_directories(RedPandaIDE PRIVATE
    ../libs/lua
    ../libs/qsynedit
    ../libs/redpanda_qt_utils
)

target_compile_definitions(RedPandaIDE PRIVATE
    APP_NAME=\\\"RedPandaCPP\\\"
    LIBEXECDIR=\\\"\\\"
)

target_link_libraries(RedPandaIDE PRIVATE
    # Remove: L${CMAKE_CURRENT_BINARY_DIR}/../libs/lua/
    # Remove: L${CMAKE_CURRENT_BINARY_DIR}/../libs/qsynedit/
    # Remove: L${CMAKE_CURRENT_BINARY_DIR}/../libs/redpanda_qt_utils/
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Svg
    Qt::Widgets
    Qt::Xml
    lua
    qsynedit
    redpanda_qt_utils
)


# Resources:
set_source_files_properties("resources/fonts/asciicontrol.ttf"
    PROPERTIES QT_RESOURCE_ALIAS "asciicontrol.ttf"
)
set(fonts_resource_files
    "resources/fonts/asciicontrol.ttf"
)

qt_add_resources(RedPandaIDE "fonts"
    PREFIX
        "/fonts"
    FILES
        ${fonts_resource_files}
)
set_source_files_properties("resources/formatdemo.cpp"
    PROPERTIES QT_RESOURCE_ALIAS "formatdemo.cpp"
)
set(codes_resource_files
    "resources/formatdemo.cpp"
)

qt_add_resources(RedPandaIDE "codes"
    PREFIX
        "/codes"
    FILES
        ${codes_resource_files}
)
set_source_files_properties("resources/autolink-xdg.json"
    PROPERTIES QT_RESOURCE_ALIAS "autolink-xdg.json"
)
set_source_files_properties("resources/autolink.json"
    PROPERTIES QT_RESOURCE_ALIAS "autolink.json"
)
set_source_files_properties("resources/codesnippets.json"
    PROPERTIES QT_RESOURCE_ALIAS "codesnippets.json"
)
set_source_files_properties("resources/terminal-unix.json"
    PROPERTIES QT_RESOURCE_ALIAS "terminal-unix.json"
)
set_source_files_properties("resources/terminal-windows.json"
    PROPERTIES QT_RESOURCE_ALIAS "terminal-windows.json"
)
set(defaultconfigs_resource_files
    "resources/autolink-xdg.json"
    "resources/autolink.json"
    "resources/codesnippets.json"
    "resources/terminal-unix.json"
    "resources/terminal-windows.json"
)

qt_add_resources(RedPandaIDE "defaultconfigs"
    PREFIX
        "/config"
    FILES
        ${defaultconfigs_resource_files}
)

qt_add_resources(RedPandaIDE "icons"
    PREFIX
        "/icons"
    FILES
        "images/associations/IDE2.ico"
        "images/associations/c.ico"
        "images/associations/cpp.ico"
        "images/associations/dev.ico"
        "images/associations/devpackage.ico"
        "images/associations/devpak.ico"
        "images/associations/h.ico"
        "images/associations/hpp.ico"
        "images/associations/ide.ico"
        "images/associations/o.ico"
        "images/associations/obj.ico"
        "images/associations/rc.ico"
        "images/associations/template.ico"
        "images/dark-close.svg"
        "images/devcpp.ico"
)

# Welcome screen theme preview
set_source_files_properties("images/demos/dark.png"
    PROPERTIES QT_RESOURCE_ALIAS "dark.png"
)
set_source_files_properties("images/demos/light.png"
    PROPERTIES QT_RESOURCE_ALIAS "light.png"
)
qt_add_resources(RedPandaIDE "icons1"
    PREFIX
        "/demos"
    FILES
        "images/demos/dark.png"
        "images/demos/light.png"
)

# Built-in empty project template
set_source_files_properties("resources/templates/empty.ico"
    PROPERTIES QT_RESOURCE_ALIAS "empty.ico"
)

set_source_files_properties("resources/templates/empty.template"
    PROPERTIES QT_RESOURCE_ALIAS "empty.template"
)

qt_add_resources(RedPandaIDE "projecttemplates"
    PREFIX
        "/templates"
    FILES
        "resources/templates/empty.ico"
        "resources/templates/empty.template"
)

file(
    GLOB
        resource_iconsets_files
    RELATIVE
        "resources/iconsets/*.json"
        "resources/iconsets/*.svg"
)

qt_add_resources(RedPandaIDE "qmake_iconsets_files"
    PREFIX
        "/"
    FILES
        ${resource_iconsets_files}
)

file(GLOB resource_theme_files
    RELATIVE
        "resources/themes/*.json"
        "resources/themes/*.lua"
        "resources/themes/*.png"
)

qt_add_resources(RedPandaIDE "qmake_theme_files"
    PREFIX
        "/"
    FILES
        ${resource_theme_files}
)

set(qmake_colorscheme_files_resource_files
    "resources/colorschemes/*.scheme,"
)

qt_add_resources(RedPandaIDE "qmake_colorscheme_files"
    PREFIX
        "/"
    FILES
        ${qmake_colorscheme_files_resource_files}
)

if(WIN32)
    target_sources(RedPandaIDE PUBLIC
        settingsdialog/environmentfileassociationwidget.cpp settingsdialog/environmentfileassociationwidget.h settingsdialog/environmentfileassociationwidget.ui
        settingsdialog/projectversioninfowidget.cpp settingsdialog/projectversioninfowidget.h settingsdialog/projectversioninfowidget.ui
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        _WIN32_WINNT=0x0501
    )

    target_link_libraries(RedPandaIDE PRIVATE
        advapi32
        psapi
        shlwapi
        user32
    )
endif()

if(QMAKE_HOST.arch___contains___x86_64)
    target_compile_definitions(RedPandaIDE PRIVATE
        ARCH_X86_64=1
    )
endif()

if((NOT (QMAKE_HOST.arch___contains___x86_64)) AND (QMAKE_HOST.arch___contains___i386))
    target_compile_definitions(RedPandaIDE PRIVATE
        ARCH_X86=1
    )
endif()

if((NOT (QMAKE_HOST.arch___contains___x86_64)) AND (QMAKE_HOST.arch___contains___i686))
    target_compile_definitions(RedPandaIDE PRIVATE
        ARCH_X86=1
    )
endif()

if(MACOS)
    target_link_libraries(RedPandaIDE PRIVATE
        Qt::GuiPrivate
    )
endif()

if(NOT APP_VERSION_SUFFIX_ISEMPTY)
    target_compile_definitions(RedPandaIDE PRIVATE
        APP_VERSION_SUFFIX=\\\"alpha\\\"
    )
endif()

if(TEST_VERSION_ISEMPTY)
    target_compile_definitions(RedPandaIDE PRIVATE
        REDPANDA_CPP_VERSION=\\\"3.4\\\"
    )
endif()

if(NOT TEST_VERSION_ISEMPTY)
    target_compile_definitions(RedPandaIDE PRIVATE
        REDPANDA_CPP_VERSION=\\\"3.4.\\\"
    )
endif()

if(WIN32 AND _WINDOWS_PREFER_OPENCONSOLE___equals___ON)
    target_compile_definitions(RedPandaIDE PRIVATE
        WINDOWS_PREFER_OPENCONSOLE
    )
endif()

if(MSVC)
    target_compile_definitions(RedPandaIDE PRIVATE
        NOMINMAX
    )
endif()

if(ENABLE_SDCC)
    target_sources(RedPandaIDE PUBLIC
        compiler/sdccfilecompiler.cpp compiler/sdccfilecompiler.h
        compiler/sdccprojectcompiler.cpp compiler/sdccprojectcompiler.h
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_SDCC
    )
endif()

if(ENABLE_LUA_ADDON)
    target_sources(RedPandaIDE PUBLIC
        addon/luaapi.cpp addon/luaapi.h
        addon/luaexecutor.cpp addon/luaexecutor.h
        addon/luaruntime.cpp addon/luaruntime.h
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_LUA_ADDON
    )
endif()

if(ENABLE_VCS)
    target_sources(RedPandaIDE PUBLIC
        settingsdialog/toolsgitwidget.cpp settingsdialog/toolsgitwidget.h settingsdialog/toolsgitwidget.ui
        vcs/gitbranchdialog.cpp vcs/gitbranchdialog.h vcs/gitbranchdialog.ui
        vcs/gitfetchdialog.cpp vcs/gitfetchdialog.h vcs/gitfetchdialog.ui
        vcs/gitlogdialog.cpp vcs/gitlogdialog.h vcs/gitlogdialog.ui
        vcs/gitmanager.cpp vcs/gitmanager.h
        vcs/gitmergedialog.cpp vcs/gitmergedialog.h vcs/gitmergedialog.ui
        vcs/gitpulldialog.cpp vcs/gitpulldialog.h vcs/gitpulldialog.ui
        vcs/gitpushdialog.cpp vcs/gitpushdialog.h vcs/gitpushdialog.ui
        vcs/gitremotedialog.cpp vcs/gitremotedialog.h vcs/gitremotedialog.ui
        vcs/gitrepository.cpp vcs/gitrepository.h
        vcs/gitresetdialog.cpp vcs/gitresetdialog.h vcs/gitresetdialog.ui
        vcs/gituserconfigdialog.cpp vcs/gituserconfigdialog.h vcs/gituserconfigdialog.ui
        vcs/gitutils.cpp vcs/gitutils.h
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_VCS
    )
endif()

if(LINUX)
    target_link_libraries(RedPandaIDE PRIVATE
        dl
        rt
    )
endif()

if(LINUX AND QT_FEATURE_static)
    target_sources(RedPandaIDE PUBLIC
        resources/linux_static_ime_plugin.cpp
    )

    target_link_libraries(RedPandaIDE PRIVATE
        # Remove: L$$[QT_INSTALL_PLUGINS]/platforminputcontexts
    )
endif()

if((QT_FEATURE_static) AND (NOT qt_translation_files.files_ISEMPTY))

    # Resources:
    set(qmake_immediate_resource_files
        "qt_translation_files"
    )

    qt_add_resources(RedPandaIDE "qmake_immediate"
        PREFIX
            "/"
        FILES
            ${qmake_immediate_resource_files}
    )
endif()

install(TARGETS RedPandaIDE
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
