// intl: lightweighted gettext MO file support based on C23 `#embed`.
// This file is in the public domain.

#pragma once

#if 1 || defined (__has_embed)

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

extern std::map<std::string_view, std::string_view> mo_data;

class intl_translator {
    using sv = std::string_view;

  public:
    intl_translator() {
        auto langs = get_language_list();

        for (auto lang : langs) {
            if (mo_data.find(lang) != mo_data.end() && load_mo_file(mo_data[lang]))
                return;
        }
    }
    intl_translator(const intl_translator &) = delete;
    intl_translator &operator=(const intl_translator &) = delete;

    const char *gettext(const char *orig)
    {
        if (auto it = translations.find(orig); it != translations.end())
            return it->second.data();
        else
            return orig;
    }

  private:
    std::map<sv, sv> translations;

  private:
    struct mo_header {
        uint32_t magic;
        uint32_t version;
        uint32_t nStrings;
        uint32_t offsetOrig;
        uint32_t offsetTrans;
        uint32_t sizeHash;
        uint32_t offSetHash;
    };

    struct mo_str {
        uint32_t length;
        uint32_t offset;

        bool valid(sv mo_file) const
        {
            return offset + length <= mo_file.size();
        }

        sv to_sv(sv &mo_file) const
        {
            return {mo_file.data() + offset, length};
        }
    };

    bool load_mo_file(sv mo_file) {
        if (mo_file.size() < sizeof(mo_header))
            return false;

        const mo_header *header = (const mo_header *)mo_file.data();
        if (header->magic != 0x950412de)
            return false;
        if (header->version != 0)
            return false;

        if (mo_file.size() < header->offsetOrig + header->nStrings * 8)
            return false;
        const mo_str *origs = (const mo_str *)(mo_file.data() + header->offsetOrig);

        if (mo_file.size() < header->offsetTrans + header->nStrings * 8)
            return false;
        const mo_str *trans = (const mo_str *)(mo_file.data() + header->offsetTrans);

        std::map<sv, sv> translations;
        for (uint32_t i = 0; i < header->nStrings; i++) {
            if (!origs[i].valid(mo_file) || !trans[i].valid(mo_file))
                return false;

            translations.insert({origs[i].to_sv(mo_file), trans[i].to_sv(mo_file)});
        }

        this->translations.swap(translations);
        return true;
    }

    struct lang_code {
        std::string lang;
        std::string region;
        std::string script;

        std::string to_gnu() const {
            std::string result = lang;
            if (!region.empty())
                result += "_" + region;
            if (!script.empty())
                result += "@" + script;
            return result;
        }

        std::vector<std::string> extended_gnu() const
        {
            std::vector<std::string> result;
            if (!script.empty()) {
                if (!region.empty())
                    result.push_back(to_gnu());        // 1. perfect match
                result.push_back(lang + "@" + script); // 2. lang & script
            }
            if (!region.empty())
                result.push_back(lang + "_" + region); // 3. lang & region
            result.push_back(lang);                    // 4. lang
            return result;
        }

        static lang_code from_gnu(std::string lang_str)
        {
            // remove encoding: zh_CN.UTF-8
            auto pos3 = lang_str.find('.');
            if (pos3 != std::string::npos)
                lang_str.erase(pos3);

            lang_code result;
            auto pos2 = lang_str.find('@');
            if (pos2 != std::string::npos) {
                result.script = lang_str.substr(pos2 + 1);
                lang_str = lang_str.substr(0, pos2);
            }
            auto pos1 = lang_str.find('_');
            if (pos1 != std::string::npos) {
                result.region = lang_str.substr(pos1 + 1);
                lang_str = lang_str.substr(0, pos1);
            }
            result.lang = lang_str;
            return result;
        }

        static lang_code from_win32(std::string langStr)
        {
            // ref.
            // https://learn.microsoft.com/en-us/windows/win32/intl/language-names

            // remove supplemental: en-US-x-fabricam
            if (auto pos = langStr.find("-x-"); pos != std::string::npos)
                langStr.erase(pos);

            lang_code result;
            auto pos1 = langStr.find("-");
            result.lang = langStr.substr(0, pos1);
            if (pos1 == std::string::npos)
                return result; // neutral

            langStr = langStr.substr(pos1 + 1);
            auto pos2 = langStr.find("-");
            if (pos2 != std::string::npos) {
                result.script = langStr.substr(0, pos2);
                result.script[0] = toupper(result.script[0]);
                result.region = langStr.substr(pos2 + 1);
            } else
                result.region = langStr;
            return result;
        }
    };

    static std::vector<std::string> get_language_list() {
        std::vector<std::string> result;
        std::set<std::string> added;

        auto add = [&result, &added](const lang_code &lc) {
            for (auto lang : lc.extended_gnu())
                if (added.find(lang) == added.end()) {
                    result.push_back(lang);
                    added.insert(lang);
                }
        };

        // LANGUAGE=zh_CN:en_US
        if (const char *langStr = getenv("LANGUAGE"); langStr) {
            auto langs = split(langStr);
            for (auto lang : langs)
                add(lang_code::from_gnu(lang));
        }

        // LC_ALL=zh_CN.UTF-8
        if (const char *lang = getenv("LC_ALL"); lang)
            add(lang_code::from_gnu(lang));

        // LC_MESSAGES=zh_CN.UTF-8
        if (const char *lang = getenv("LC_MESSAGES"); lang)
            add(lang_code::from_gnu(lang));

        // LANG=zh_CN.UTF-8
        if (const char *lang = getenv("LANG"); lang)
            add(lang_code::from_gnu(lang));

#ifdef _WIN32
        for (auto lang : get_win32_language_list())
            add(lang_code::from_win32(lang));
#endif

        return result;
    }

    static std::vector<std::string> split(const std::string &str,
                                          char sep = ':') {
        std::vector<std::string> result;
        size_t pos = 0;
        while (pos < str.size()) {
            size_t next = str.find(sep, pos);
            if (next == std::string::npos)
                next = str.size();
            size_t length = next - pos;
            if (length > 0)
                result.push_back(str.substr(pos, length));
            pos = next + 1;
        }
        return result;
    }

#ifdef _WIN32
    static std::vector<std::string> get_win32_language_list() {
        static auto pGetUserPreferredUILanguages =
            reinterpret_cast<decltype(&GetUserPreferredUILanguages)>(
                GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
                               "GetUserPreferredUILanguages"));
        if (!pGetUserPreferredUILanguages)
            return {};

        constexpr int buf_len = 4096;
        wchar_t wbuf[buf_len];
        ULONG len = buf_len;
        ULONG num_langs;
        if (!pGetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &num_langs, wbuf, &len))
            return {};

        char buf[buf_len];
        for (int i = 0; i < len; i++)
            buf[i] = wbuf[i];

        std::vector<std::string> result;
        char *pos = buf;
        while (*pos) {
            std::string lang = std::string(pos);
            pos += lang.size() + 1;
            result.push_back(lang);
        }
        return result;
    }
#endif
};

inline const char *_(const char *orig)
{
    static intl_translator translator;
    return translator.gettext(orig);
}

#else

inline const char *_(const char *orig)
{
    return orig;
}

#endif
