target("better-enums")
    set_kind("phony")

    add_headerfiles("enum.h")
    add_includedirs(".", {public = true})

    -- do not install
    on_install(function (target) end)
