global function apiVersion(): ApiVersion
    return {
        kind = "theme",
        major = 0,
        minor = 1,
    }
end

-- Follow KDE naming
local nameMap: {string:string} = {
    en_US = "Breeze Dark",
    pt_BR = "Brisa Escura",
    zh_CN = "微风深色",
    zh_TW = "Breeze Dark",
}

global function main(): Theme
    local function getStyle(): string
        return "MiniBreeze";
        local styles = C_Desktop.qtStyleList()
        -- prefer geniune breeze
        for _, style in ipairs(styles) do
            if style == "breeze" then
                return "breeze"
            end
        end
        return "MiniBreeze"
    end

    local lang = C_Desktop.language()

    return {
        ["name"] = nameMap[lang] or nameMap.en_US,
        ["style"] = getStyle(),
        ["default scheme"] = "Adaptive",
        ["default iconset"] = "newlook",
        ["palette"] = {},
    }
end
