export const Debug = {
    log(text) {},
    dump(obj) {},
}

export const Desktop = {
    desktopEnvironment() { return "xdg"; },
    language() { return "en_US"; },
    qtStyleList() { return ["breeze", "fusion"]; },
    systemAppMode() { return "dark"; },
};
