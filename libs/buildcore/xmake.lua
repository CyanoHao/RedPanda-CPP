target("buildcore")
    add_rules("qt.static")
    add_deps("redpanda_qt_utils")

    add_files(
        "toolchain/compilerinfo.cpp")
    add_includedirs(".", {public = true})

    add_links("redpanda_qt_utils")

    -- do not install
    on_install(function (target) end)
