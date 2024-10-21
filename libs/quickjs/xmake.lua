target("quickjs")
    set_kind("static")

    add_cflags("-fwrapv")

    add_defines(
        "_GNU_SOURCE",
        'CONFIG_VERSION="2024-01-13"',
        "CONFIG_BIGNUM")

    if is_plat("mingw") then
        add_defines("__USE_MINGW_ANSI_STDIO")
    end

    add_files(
        "quickjs/cutils.c",
        "quickjs/libbf.c",
        "quickjs/libregexp.c",
        "quickjs/libunicode.c",
        "quickjs/quickjs.c")

    add_includedirs(".", {public = true})
    add_headerfiles("quickjs/quickjs.h")

    -- do not install
    on_install(function (target) end)
