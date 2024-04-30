target("minikf")
    add_rules("qt.static")
    add_frameworks("QtGui", "QtWidgets")

    add_files(
        "KGuiAddons/kcolorspaces.cpp",
        "KGuiAddons/kcolorutils.cpp")

    add_includedirs(
        "KGuiAddons",
        {public = true})

    -- do not install
    on_install(function (target) end)
