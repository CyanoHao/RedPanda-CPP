target("ced")
    set_kind("static")

    add_files(
        "compact_enc_det/compact_enc_det.cc",
        "compact_enc_det/compact_enc_det_hint_code.cc",
        "util/encodings/encodings.cc",
        "util/languages/languages.cc")
    add_includedirs(".", {public = true})

    -- do not install
    on_install(function (target) end)
