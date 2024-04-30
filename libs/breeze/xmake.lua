target("breeze")
    add_rules("qt.static")
    add_frameworks("QtGui", "QtWidgets")

    add_includedirs(
        "breeze",
        "breeze/animations")

    add_moc_classes(
        "breeze/breezestyle")

    add_includedirs(".", {public = true})

    -- do not install
    on_install(function (target) end)
