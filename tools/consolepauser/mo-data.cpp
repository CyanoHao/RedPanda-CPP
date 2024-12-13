// This file is in the public domain.

#ifdef __has_embed

#include <map>
#include <string_view>

unsigned char raw_zh_CN[] = {
    #embed "mo/zh_CN.mo"
};

unsigned char raw_zh_TW[] = {
    #embed "mo/zh_TW.mo"
};

std::map<std::string_view, std::string_view> mo_data = {
    { "zh_CN", { (char *)raw_zh_CN, sizeof(raw_zh_CN) } },
    { "zh_TW", { (char *)raw_zh_TW, sizeof(raw_zh_TW) } },
};

#endif
