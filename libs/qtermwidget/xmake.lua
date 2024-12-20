target("qtermwidget")
    add_rules("qt.static")
    add_rules("qt.ts")
    add_frameworks("QtGui", "QtWidgets")

    add_defines(
        'KB_LAYOUT_DIR=":/qtermwidget/kb-layouts"',
        'COLORSCHEMES_DIR=":/qtermwidget/color-schemes"',
        'TRANSLATIONS_DIR=":/qtermwidget/translations"',
        "HAVE_POSIX_OPENPT",
        "HAVE_SYS_TIME_H")

    add_files(
        "qtermwidget/BlockArray.cpp",
        "qtermwidget/ColorScheme.cpp",
        "qtermwidget/History.cpp",
        "qtermwidget/KeyboardTranslator.cpp",
        "qtermwidget/konsole_wcwidth.cpp",
        "qtermwidget/kpty.cpp",
        "qtermwidget/Screen.cpp",
        "qtermwidget/ShellCommand.cpp",
        "qtermwidget/TerminalCharacterDecoder.cpp",
        "qtermwidget/tools.cpp")

    add_moc_classes(
        "qtermwidget/Emulation",
        "qtermwidget/Filter",
        "qtermwidget/HistorySearch",
        "qtermwidget/kprocess",
        "qtermwidget/kptyprocess",
        "qtermwidget/Pty",
        "qtermwidget/qtermwidget",
        "qtermwidget/ScreenWindow",
        "qtermwidget/SearchBar",
        "qtermwidget/Session",
        "qtermwidget/TerminalDisplay",
        "qtermwidget/Vt102Emulation")

    add_moc_classes_with_private_slots(
        "qtermwidget/kptydevice")

    add_ui_classes(
        "qtermwidget/SearchBar")

    add_includedirs("qtermwidget")
    add_includedirs(".", {public = true})

    -- do not install
    on_install(function (target) end)
