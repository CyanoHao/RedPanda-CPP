export const apiVersion = {
    major: 0,
    minor: 1,
};

export default function () {

    const locale = "";

    const localizedName = new Map([
        ["en_US", "Auto (follow system style and color)"],
        ["zh_CN", "自动（跟随系统样式和颜色）"],
        ["zh_TW", "自動（跟隨系統樣式和顏色）"],
        ["pt_BR", "Automático (seguir estilo e cor do sistema)"],
    ]);

    return {
        "name": localizedName.get(locale) || localizedName.get("en_US"),
        "style": "breeze",
        "default scheme": "Adaptive",
        "default iconset": "newlook",
        "palette": {}
    };
};
