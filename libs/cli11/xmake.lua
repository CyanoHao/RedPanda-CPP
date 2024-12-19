target("cli11")
    set_kind("phony")

    add_includedirs(".", {public = true})
    add_headerfiles(
        "CLI/*.hpp",
        "CLI/impl/*.hpp")

    -- do not install
    on_install(function (target) end)
