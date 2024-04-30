global function apiVersion(): ApiVersion
    return {
        kind = "theme",
        major = 0,
        minor = 1,
    }
end

local nameMap: {string:string} = {
    en_US = "Fusion (auto)",
    pt_BR = "Fusão (automática)",
    zh_CN = "融合（自动）",
    zh_TW = "融合（自動）",
}

global function main(): Theme
    local systemAppMode = C_Desktop.systemAppMode()
    local isDarkMode = systemAppMode == "dark"

    local function getStyle(): string
        if isDarkMode then
            return "RedPandaDarkFusion"
        else
            return "RedPandaLightFusion"
        end
    end

    local function getPalette(): Theme.Palette
        if isDarkMode then
            return { -- palette from `dark.lua`
                PaletteWindow = "#19232D",
                PaletteWindowText = "#E0E1E3",
                PaletteBase = "#1E1E1E",
                PaletteAlternateBase = "#303030",
                PaletteButton = "#19232D",
                PaletteButtonDisabled = "#19232D",
                PaletteBrightText = "#ff0000",
                PaletteText = "#e7e7e7",
                PaletteButtonText = "#d3d3d3",
                PaletteButtonTextDisabled = "#9DA9B5",
                PaletteHighlight = "#aa1f75cc",
                PaletteDark = "#232323",
                PaletteHighlightedText = "#e7e7e7",
                PaletteToolTipBase = "#66000000",
                PaletteToolTipText = "#e7e7e7",
                PaletteLink = "#007af4",
                PaletteLinkVisited = "#a57aff",
                PaletteWindowDisabled = "#333333",
                PaletteWindowTextDisabled = "#9DA9B5",
                PaletteHighlightDisabled = "#26486B",
                PaletteHighlightedTextDisabled = "#9DA9B5",
                PaletteBaseDisabled = "#19232D",
                PaletteTextDisabled = "#9DA9B5",
                PaletteMid = "#707070",
                PaletteLight = "#505050",
                PaletteMidlight = "#00ff00",
            }
        else
            return { -- palette from `default.lua`
                PaletteWindow = "#efefef",
                PaletteWindowText = "#000000",
                PaletteBase = "#ffffff",
                PaletteAlternateBase = "#f7f7f7",
                PaletteToolTipBase = "#ffffdc",
                PaletteToolTipText = "#000000",
                PaletteText = "#000000",
                PaletteButton = "#efefef",
                PaletteButtonText = "#000000",
                PaletteBrightText = "#ffffff",
                PaletteLink = "#0000ff",
                PaletteLinkVisited = "#ff00ff",
                PaletteLight = "#ffffff",
                PaletteMidlight = "#cacaca",
                PaletteDark = "#9f9f9f",
                PaletteMid = "#b8b8b8",
                PaletteWindowDisabled = "#efefef",
                PaletteWindowTextDisabled = "#bebebe",
                PaletteBaseDisabled = "#efefef",
                PaletteTextDisabled = "#bebebe",
                PaletteButtonDisabled = "#efefef",
                PaletteButtonTextDisabled = "#bebebe",
                PaletteHighlight = "#dddddd",
                PaletteHighlightedText = "#000000",
            }
        end
    end

    local lang = C_Desktop.language()

    return {
        ["name"] = nameMap[lang] or nameMap.en_US,
        ["style"] = getStyle(),
        ["default scheme"] = "Adaptive",
        ["default iconset"] = "newlook",
        ["palette"] = getPalette()
    }
end
