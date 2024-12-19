target("consolepauser")
    set_kind("binary")

    add_options("mingw-static")

    add_deps("cli11")

    if is_os("windows") then
        add_defines(
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX")
        add_files("main.windows.cpp")
        if is_plat("mingw") then
            add_ldflags("-municode", {force = true})
        end
    else
        add_files("main.unix.cpp")
    end

    if is_os("windows") then
        add_syslinks("psapi")
    elseif is_os("linux") then
        add_syslinks("rt")
    end

    if is_xdg() then
        on_install(install_libexec)
    end
