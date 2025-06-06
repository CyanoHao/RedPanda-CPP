add_executable(RedPandaIDE WIN32 MACOSX_BUNDLE
    autolinkmanager.cpp
    colorscheme.cpp
    customfileiconprovider.cpp
    projectoptions.cpp
    settings.cpp
    syntaxermanager.cpp
    systemconsts.cpp
    utils.cpp
    visithistorymanager.cpp
    # compiler
    compiler/compilerinfo.cpp
    # debugger
    debugger/dapprotocol.cpp
    debugger/gdbmiresultparser.cpp
    # parser
    parser/cppparser.cpp
    parser/cpppreprocessor.cpp
    parser/cpptokenizer.cpp
    parser/parserutils.cpp
    parser/statementmodel.cpp
    # problems
    problems/competitivecompenionhandler.cpp
    problems/freeprojectsetformat.cpp
    problems/ojproblemset.cpp
    problems/problemcasevalidator.cpp
    # utils
    utils/escape.cpp
    utils/font.cpp
    utils/parsearg.cpp
)

target_moc_classes(RedPandaIDE
    caretlist
    codesnippetsmanager
    cpprefacter
    editor
    editorlist
    iconsmanager
    main
    project
    projecttemplate
    shortcutmanager
    symbolusagemanager
    thememanager
    todoparser
    toolsmanager
    # compiler
    compiler/compiler
    compiler/compilermanager
    compiler/executablerunner
    compiler/filecompiler
    compiler/ojproblemcasesrunner
    compiler/projectcompiler
    compiler/runner
    compiler/stdincompiler
    # debugger
    debugger/dapdebugger
    debugger/debugger
    debugger/gdbmidebugger
    # parser
    parser/cppparser
    parser/statementmodel
    # problems
    problems/competitivecompenionhandler
    # settings dialog
    settingsdialog/settingswidget
    # widgets
    widgets/bookmarkmodel
    widgets/classbrowser
    widgets/codecompletionlistview
    widgets/codecompletionpopup
    widgets/coloredit
    widgets/compileargumentswidget
    widgets/customdisablediconengine
    widgets/customfilesystemmodel
    widgets/darkfusionstyle
    widgets/editorstabwidget
    widgets/filenameeditdelegate
    widgets/functiontooltipwidget
    widgets/headercompletionpopup
    widgets/issuestable
    widgets/labelwithmenu
    widgets/lightfusionstyle
    widgets/linenumbertexteditor
    widgets/macroinfomodel
    widgets/ojproblemsetmodel
    widgets/qconsole
    widgets/qpatchedcombobox
    widgets/searchresultview
    widgets/shortcutinputedit
    widgets/shrinkabletabwidget
)

target_ui_classes(RedPandaIDE
    mainwindow
    # settings dialog
    settingsdialog/compilerautolinkwidget
    settingsdialog/compilersetdirectorieswidget
    settingsdialog/compilersetoptionwidget
    settingsdialog/debuggeneralwidget
    settingsdialog/editorautosavewidget
    settingsdialog/editorclipboardwidget
    settingsdialog/editorcodecompletionwidget
    settingsdialog/editorcolorschemewidget
    settingsdialog/editorcustomctypekeywords
    settingsdialog/editorfontwidget
    settingsdialog/editorgeneralwidget
    settingsdialog/editormiscwidget
    settingsdialog/editorsnippetwidget
    settingsdialog/editorsymbolcompletionwidget
    settingsdialog/editorsyntaxcheckwidget
    settingsdialog/editortooltipswidget
    settingsdialog/environmentappearancewidget
    settingsdialog/environmentfolderswidget
    settingsdialog/environmentperformancewidget
    settingsdialog/environmentprogramswidget
    settingsdialog/environmentshortcutwidget
    settingsdialog/executorgeneralwidget
    settingsdialog/executorproblemsetwidget
    settingsdialog/formattergeneralwidget
    settingsdialog/formatterpathwidget
    settingsdialog/languageasmgenerationwidget
    settingsdialog/projectcompileparamaterswidget
    settingsdialog/projectcompilerwidget
    settingsdialog/projectdirectorieswidget
    settingsdialog/projectdllhostwidget
    settingsdialog/projectfileswidget
    settingsdialog/projectgeneralwidget
    settingsdialog/projectmakefilewidget
    settingsdialog/projectoutputwidget
    settingsdialog/projectprecompilewidget
    settingsdialog/settingsdialog
    settingsdialog/toolsgeneralwidget
    # widgets
    widgets/aboutdialog
    widgets/choosethemedialog
    widgets/cpudialog
    widgets/custommakefileinfodialog
    widgets/editorfontdialog
    widgets/filepropertiesdialog
    widgets/infomessagebox
    widgets/newclassdialog
    widgets/newheaderdialog
    widgets/newprojectdialog
    widgets/newprojectunitdialog
    widgets/newtemplatedialog
    widgets/ojproblempropertywidget
    widgets/projectalreadyopendialog
    widgets/searchdialog
    widgets/searchinfiledialog
    widgets/signalmessagedialog
)

if(LUA_ADDON)
    target_sources(RedPandaIDE PUBLIC
        addon/luaapi.cpp
        addon/luaexecutor.cpp
        addon/luaruntime.cpp
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_LUA_ADDON
    )

    target_link_libraries(RedPandaIDE PRIVATE
        lua
    )
endif()

if(SDCC)
    target_sources(RedPandaIDE PUBLIC
        compiler/sdccfilecompiler.cpp
        compiler/sdccprojectcompiler.cpp
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_SDCC
    )
endif()

if(VCS)
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

target_compile_definitions(RedPandaIDE PRIVATE
    APP_NAME=\"${APP_NAME}\"
    LIBEXECDIR=\"${LIBEXECDIR}\"
)

target_include_directories(RedPandaIDE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RedPandaIDE PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Svg
    Qt::Widgets
    Qt::Xml
    qsynedit
    redpanda_qt_utils
)

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
    PREFIX "/config"
    FILES
        ${defaultconfigs_resource_files}
)

qt_add_resources(RedPandaIDE "resource_icons"
    PREFIX "/icons"
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
qt_add_resources(RedPandaIDE "resource_demos"
    PREFIX "/demos"
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
qt_add_resources(RedPandaIDE "resource_projecttemplates"
    PREFIX "/templates"
    FILES
        "resources/templates/empty.ico"
        "resources/templates/empty.template"
)

file(GLOB resource_iconsets_files
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "resources/iconsets/*.json"
    "resources/iconsets/*.svg"
)
qt_add_resources(RedPandaIDE "resource_iconsets"
    PREFIX "/"
    FILES ${resource_iconsets_files}
)

file(GLOB resource_theme_files
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "resources/themes/*.json"
    "resources/themes/*.lua"
    "resources/themes/*.png"
)
qt_add_resources(RedPandaIDE "resource_theme"
    PREFIX "/"
    FILES ${resource_theme_files}
)

file(GLOB resource_colorscheme_files
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "resources/colorschemes/*.scheme"
)
qt_add_resources(RedPandaIDE "resource_colorscheme"
    PREFIX "/"
    FILES ${resource_colorscheme_files}
)

if(WIN32)
    target_ui_classes(RedPandaIDE
        settingsdialog/environmentfileassociationwidget
        settingsdialog/projectversioninfowidget
    )

    target_link_libraries(RedPandaIDE PRIVATE
        advapi32
        psapi
        shlwapi
        user32
    )
endif()

target_compile_definitions(RedPandaIDE PRIVATE
    ARCH_X86_64=1
    ARCH_X86=1
)

if(MACOS)
    target_link_libraries(RedPandaIDE PRIVATE
        Qt::GuiPrivate
    )
endif()

if(APP_VERSION_PRE_RELEASE)
    target_compile_definitions(RedPandaIDE PRIVATE
        APP_VERSION_SUFFIX=\"${APP_VERSION_PRE_RELEASE}\"
    )
endif()

target_compile_definitions(RedPandaIDE PRIVATE
    REDPANDA_CPP_VERSION=\"${REDPANDA_VERSION}\"
)

if(WINDOWS_PREFER_OPENCONSOLE)
    target_compile_definitions(RedPandaIDE PRIVATE
        WINDOWS_PREFER_OPENCONSOLE
    )
endif()

if(LINUX AND QT_FEATURE_static)
    target_sources(RedPandaIDE PUBLIC
        resources/linux_static_ime_plugin.cpp
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
