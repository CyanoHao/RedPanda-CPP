target("style")
    add_rules("qt.static")
    add_frameworks("QtGui", "QtWidgets")

    add_files("style/phantomcolor.cpp")

    add_moc_classes(
        "style/breezestyle",
        "style/phantomstyle")

    add_ui_classes()

    add_defines("PHANTOM_NO_MOC")
    add_includedirs(".", {interface = true})

    -- do not install
    on_install(function (target) end)
