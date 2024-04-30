global function apiVersion(): ApiVersion
    return {
        kind = "theme",
        major = 0,
        minor = 1,
    }
end

-- Follow KDE naming
local nameMap: {string:string} = {
    en_US = "Breeze (auto)",
    pt_BR = "Brisa (automática)",
    zh_CN = "微风（自动）",
    zh_TW = "Breeze（自动）",
}

global function main(): Theme
    local systemAppMode = C_Desktop.systemAppMode()
    local isDarkMode = systemAppMode == "dark"

    local function getStyle(): string
        local styles = C_Desktop.qtStyleList()
        -- prefer geniune breeze
        for _, style in ipairs(styles) do
            if style == "breeze" then
                return "breeze"
            end
        end
        return "MiniBreeze"
    end

    local function getPalette(): Theme.Palette
        if isDarkMode then
            return {}
        else
            return {}
        end
    end

    local lang = C_Desktop.language()

    return {
        ["name"] = nameMap[lang] or nameMap.en_US,
        ["style"] = getStyle(),
        ["default scheme"] = "Adaptive",
        ["default iconset"] = "newlook",
        ["palette"] = getPalette(),
    }
end
