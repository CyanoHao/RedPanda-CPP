function apiVersion()
   return {
      kind = "theme",
      major = 0,
      minor = 1,
   }
end


local nameMap = {
   en_US = "Breeze Dark",
   pt_BR = "Brisa Escura",
   zh_CN = "微风深色",
   zh_TW = "Breeze Dark",
}

function main()
   local function getStyle()
      return "MiniBreeze"
      local styles = C_Desktop.qtStyleList()

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
