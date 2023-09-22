import { Desktop } from 'RedPandaTheme';

export const apiVersion = {
    major: 0,
    minor: 1,
};

export default function () {

    const locale = Desktop.getLocale();

    const localizedName = new Map([
        ["en_US", "Light"],
        ["zh_CN", "浅色"],
        ["zh_TW", "淺色"],
        ["pt_BR", "Claro"],
    ]);

    return {
        "name": localizedName.get(locale, "Light"),
        "style": "RedPandaLightFusion",
        "default scheme": "Intellij Classic",
        "default iconset": "newlook",
        "palette": {
            PaletteWindow: "#efefef",
            PaletteWindowText: "#000000",
            PaletteBase: "#ffffff",
            PaletteAlternateBase: "#f7f7f7",
            PaletteToolTipBase: "#ffffdc",
            PaletteToolTipText: "#000000",
            PaletteText: "#000000",
            PaletteButton: "#efefef",
            PaletteButtonText: "#000000",
            PaletteBrightText: "#ffffff",
            PaletteLink: "#0000ff",
            PaletteLinkVisited: "#ff00ff",
            PaletteLight: "#ffffff",
            PaletteMidLight: "#cacaca",
            PaletteDark: "#9f9f9f",
            PaletteMid: "#b8b8b8",
            PaletteWindowDisabled: "#efefef",
            PaletteWindowTextDisabled: "#bebebe",
            PaletteBaseDisabled: "#efefef",
            PaletteTextDisabled: "#bebebe",
            PaletteButtonDisabled: "#efefef",
            PaletteButtonTextDisabled: "#bebebe",
            PaletteHighlight: "#ffdddddd",
            PaletteHighlightedText: "#ff000000",
        },
    };
};
