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

target_compile_definitions(RedPandaIDE PRIVATE
    APP_NAME=\"${APP_NAME}\"
    LIBEXECDIR=\"${CMAKE_INSTALL_LIBEXECDIR}\"
    ARCH_X86_64=1
    ARCH_X86=1
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

target_compile_definitions(RedPandaIDE PRIVATE
    REDPANDA_CPP_VERSION=\"${REDPANDA_VERSION}\"
)

if(APP_VERSION_PRE_RELEASE)
    target_compile_definitions(RedPandaIDE PRIVATE
        APP_VERSION_SUFFIX=\"${APP_VERSION_PRE_RELEASE}\"
    )
endif()

#####################
# Platform-specific #
#####################

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

if(MACOS)
    target_link_libraries(RedPandaIDE PRIVATE
        Qt::GuiPrivate
    )
endif()

if(LINUX AND QT_FEATURE_static)
    target_sources(RedPandaIDE PRIVATE
        resources/linux_static_ime_plugin.cpp
    )
endif()

install(TARGETS RedPandaIDE
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

###################
# Feature options #
###################

if(LUA_ADDON)
    target_sources(RedPandaIDE PRIVATE
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
    target_moc_classes(RedPandaIDE
        compiler/sdccfilecompiler
        compiler/sdccprojectcompiler
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_SDCC
    )
endif()

if(VCS)
    target_moc_classes(RedPandaIDE
        vcs/gitmanager
        vcs/gitrepository
        vcs/gitutils
    )
    target_ui_classes(RedPandaIDE
        settingsdialog/toolsgitwidget
        vcs/gitbranchdialog
        vcs/gitfetchdialog
        vcs/gitlogdialog
        vcs/gitmergedialog
        vcs/gitpulldialog
        vcs/gitpushdialog
        vcs/gitremotedialog
        vcs/gitresetdialog
        vcs/gituserconfigdialog
    )

    target_compile_definitions(RedPandaIDE PRIVATE
        ENABLE_VCS
    )
endif()

if(WINDOWS_PREFER_OPENCONSOLE)
    target_compile_definitions(RedPandaIDE PRIVATE
        WINDOWS_PREFER_OPENCONSOLE
    )
endif()

#############
# Resources #
#############

target_sources(RedPandaIDE PRIVATE
    fonts.qrc
    codes.qrc
    defaultconfigs.qrc
    icons.qrc
    projecttemplates.qrc
)

# translation

target_embed_translations(RedPandaIDE
    translations/RedPandaIDE_pt_BR.ts
    translations/RedPandaIDE_ru_RU.ts
    translations/RedPandaIDE_zh_CN.ts
    translations/RedPandaIDE_zh_TW.ts
)

if(QT_FEATURE_static)
    set(qtbase_translation_filenames
        qtbase_pt_BR.qm
        qtbase_ru_RU.qm
        qtbase_zh_CN.qm
        qtbase_zh_TW.qm
    )

    set(qtbase_translation_files)

    foreach(filename IN LIST qtbase_translation_filenames)
        if(EXISTS ${QT6_INSTALL_TRANSLATIONS}/${filename})
            list(APPEND qtbase_translation_files ${QT6_INSTALL_TRANSLATIONS}/${filename})
        endif()
    endforeach()

    target_embed_resources(RedPandaIDE "qtbase_translation"
        ${qtbase_translation_files}
    )
endif()

# appereance

file(GLOB_RECURSE resource_iconset_files
    "resources/iconsets/*.json"
    "resources/iconsets/*.svg"
)
target_embed_resources(RedPandaIDE iconsets
    ${resource_iconset_files}
)

file(GLOB resource_theme_files
    "resources/themes/*.json"
    "resources/themes/*.lua"
    "resources/themes/*.png"
)
target_embed_resources(RedPandaIDE theme
    ${resource_theme_files}
)

file(GLOB resource_colorscheme_files
    "resources/colorschemes/*.scheme"
)
target_embed_resources(RedPandaIDE colorschemes
    ${resource_colorscheme_files}
)

# app icon

if(WIN32)
    target_sources(RedPandaIDE PRIVATE
        resources/windows_icon.rc
    )
elseif(MACOS)
    set(app_icon ../platform/macos/RedPandaIDE.icns)
    set_source_files_properties(${app_icon}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(RedPandaIDE PRIVATE
        ${app_icon}
    )
endif()
