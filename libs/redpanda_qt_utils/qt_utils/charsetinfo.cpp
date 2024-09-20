/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "charsetinfo.h"
#include <QObject>
#include <memory>
#include <QMap>
#include <QSet>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "utils.h"

QByteArray CharsetInfoManager::getDefaultSystemEncoding()
{
#ifdef Q_OS_WIN
    DWORD acp = GetACP();
    PCharsetInfo info = findCharsetByCodepage(acp);
    if (info) {
        return info->name;
    }
    return "unknown";
#else
    return "UTF-8";
#endif
}

PCharsetInfo CharsetInfoManager::findCharsetByCodepage(int codepage)
{
    foreach (const PCharsetInfo& info, mCodePages) {
        if (info->codepage == codepage)
            return info;
    }
    return PCharsetInfo();
}

QStringList CharsetInfoManager::languageNames()
{
    QSet<QString> languages;
    foreach (const PCharsetInfo& info, mCodePages) {
        if (info->enabled)
            languages.insert(info->group);
    }
    QStringList lst;
    foreach (const QString& s, languages)
        lst.append(s);
    lst.sort(Qt::CaseInsensitive);
    return lst;
}

QList<PCharsetInfo> CharsetInfoManager::findCharsetsByLanguageName(const QString &languageName)
{
    QList<PCharsetInfo> result;
    foreach (const PCharsetInfo& info, mCodePages) {
        if (info->enabled && info->group == languageName)
            result.append(info);
    }
    std::sort(result.begin(),result.end(),[](const PCharsetInfo& info1,const PCharsetInfo& info2){
        return (info1->qtName < info2->qtName);
    });
    return result;
}

QList<PCharsetInfo> CharsetInfoManager::findCharsetByLocale(const QString &localeName)
{
    QList<PCharsetInfo> result;
    foreach (const PCharsetInfo& info, mCodePages) {
        if (info->enabled && info->localeName == localeName)
            result.append(info);
    }
    return result;
}

QString CharsetInfoManager::findLanguageByCharsetName(const QString &encodingName)
{

    foreach (const PCharsetInfo& info, mCodePages) {
        if (info->enabled &&
                QString::compare(info->qtName, encodingName, Qt::CaseInsensitive)==0)
            return info->group;
    }
    return "Unknown";
}

const QString &CharsetInfoManager::localeName() const
{
    return mLocaleName;
}

CharsetInfoManager::CharsetInfoManager(const QString& localeName):
    QObject(),
    mLocaleName(localeName)
{
    using std::nullopt;
    const QString ACP = tr("Windows ANSI code page");

    mCodePages = {
        // Unicode
        {"UTF-8", "UTF-8", "UTF-8", {"Unicode", ACP}, {}, 65001},
        {"UTF-16LE", "UTF-16LE", tr("UTF-16 little endian"), {"Unicode"}, {}, nullopt},
        {"UTF-16BE", "UTF-16BE", tr("UTF-16 big endian"), {"Unicode"}, {}, nullopt},
        {"UTF-32LE", "UTF-32LE", tr("UTF-32 little endian"), {"Unicode"}, {}, nullopt},
        {"UTF-32BE", "UTF-32BE", tr("UTF-32 big endian"), {"Unicode"}, {}, nullopt},
        {"GB18030", "GB18030", "GB 18030", {"Unicode", "简体中文"}, {}, nullopt},

        // Win32 ACP
        {"tis-620", "tis-620", "TIS-620", {ACP, "ภาษาไทย"}, {"th"}, 874},
        {"shift_jis", "windows-932", "Shift-JIS", {ACP, "日本語"}, {"ja"}, 932},
        {"GBK", "GBK", "GBK", {ACP, "简体中文"}, {"zh_CN", "zh_SG"}, 936},
        {"windows-949", "windows-949", "EUC-KR", {ACP, "한국어/조선말"}, {"ko"}, 949},
        {"big5", "big5", "Big5", {ACP, "繁體中文"}, {"zh_TW", "zh_HK"}, 950},
        {"windows-1250", "windows-1250", "Windows 1250", {ACP, tr("Central European")}, {}, 1250},
        {"windows-1251", "windows-1251", "Windows 1251", {ACP, "Кириллица"}, {}, 1251},
        {"windows-1252", "windows-1252", "Windows 1252", {ACP, tr("Western European")}, {}, 1252},
        {"windows-1253", "windows-1253", "Windows 1253", {ACP, "Ελληνικό"}, {}, 1253},
        {"windows-1254", "windows-1254", "Windows 1254", {ACP, "Türkçe"}, {}, 1254},
        {"windows-1255", "windows-1255", "Windows 1255", {ACP, "עברית"}, {}, 1255},
        {"windows-1256", "windows-1256", "Windows 1256", {ACP, "العربية"}, {}, 1256},
        {"windows-1257", "windows-1257", "Windows 1257", {ACP, tr("Baltic")}, {}, 1257},
        {"windows-1258", "windows-1258", "Windows 1258", {ACP, "Tiếng Việt"}, {}, 1258},
        // UTF-8 (since Windows 10 1607) listed above

        // ISO/IEC 8859
        {"iso-8859-1", "iso-8859-1", "ISO 8859-1", {tr("Western Europe")}, {}, nullopt},
        {"iso-8859-2", "iso-8859-2", "ISO 8859-2", {tr("Eastern Europe")}, {}, nullopt},
        {"iso-8859-3", "iso-8859-3", "ISO 8859-3", {tr("Latin 3")}, {}, nullopt},
        {"iso-8859-4", "iso-8859-4", "ISO 8859-4", {tr("Baltic")}, {}, nullopt},
        {"iso-8859-5", "iso-8859-5", "ISO 8859-5", {"Кириллица"}, {}, nullopt},
        {"iso-8859-6", "iso-8859-6", "ISO 8859-6", {"العربية"}, {}, nullopt},
        {"iso-8859-7", "iso-8859-7", "ISO 8859-7", {"Ελληνικό"}, {}, nullopt},
        {"iso-8859-8", "iso-8859-8", "ISO 8859-8", {"עברית"}, {}, nullopt},
        {"iso-8859-9", "iso-8859-9", "ISO 8859-9", {"Türkçe"}, {}, nullopt},
        {"iso-8859-10", "iso-8859-10", "ISO 8859-10", {tr("Latin 4")}, {}, nullopt},
        {"iso-8859-11", "iso-8859-11", "ISO 8859-11", {"ภาษาไทย"}, {}, nullopt},
        {"iso-8859-13", "iso-8859-13", "ISO 8859-13", {tr("Baltic")}, {}, nullopt},
        {"iso-8859-14", "iso-8859-14", "ISO 8859-14", {tr("Celtic")}, {}, nullopt},
        {"iso-8859-15", "iso-8859-15", "ISO 8859-15", {tr("Western Europe")}, {}, nullopt},
        {"iso-8859-16", "iso-8859-16", "ISO 8859-16", {tr("South-Eastern Europe")}, {}, nullopt},

        // Extended Unix Code
        {"euc-jp", "euc-jp", "EUC-JP", {"日本語"}, {}, nullopt},
        // EUC-KR is subset of CP949
        // EUC-CN is subset of GBK
    };

    mCodePages.append(std::make_shared<CharsetInfo>(10000,"macintosh",tr("Cyrillic"),"",true));
    mCodePages.append(std::make_shared<CharsetInfo>(20866,"koi8-r",tr("Cyrillic"),"",true));
    mCodePages.append(std::make_shared<CharsetInfo>(21866,"koi8-u",tr("Cyrillic"),"",true));

    mCodePages.append(std::make_shared<CharsetInfo>(51932,"euc-jp",tr("Japanese"),"",true));
    mCodePages.append(std::make_shared<CharsetInfo>(51949,"euc-kr",tr("Korean"),"",true));
}

CharsetInfo::CharsetInfo(int codepage, const QByteArray &name, const QString &language,const QString& localeName, bool enabled)
{
    this->codepage = codepage;
    this->qtName = name;
    this->group = language;
    this->localeName = localeName;
    this->enabled = enabled;
}
