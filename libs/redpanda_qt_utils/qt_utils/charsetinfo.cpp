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
#include "utils.h"
#include <QObject>
#include <memory>
#include <QMap>
#include <QSet>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

CharsetInfoManager* pCharsetInfoManager;

// based on manual enumeration on Windows 8.1
// cross validated with Wikipedia
const QList<Win32AnsiCodePage> Win32AnsiCodePage::codePageList = {
    {0, ENCODING_SYSTEM_DEFAULT, "0 - ANSI"},
    {874, "tis-620", "874 - ภาษาไทย" },
    {932, "shift_jis", "932 - 日本語"},
    {936, "gbk", "936 - 简体中文"},
    {949, "windows-949", "949 - 한국어/조선말"},
    {950, "big5", "950 - 繁體中文"},
    {1250, "windows-1250", "1250 - Latin 2"},
    {1251, "windows-1251", "1251 - Кириллица"},
    {1252, "windows-1252", "1252 - Latin 1"},
    {1253, "windows-1253", "1253 - Ελληνικό"},
    {1254, "windows-1254", "1254 - Türkçe"},
    {1255, "windows-1255", "1255 - עברית"},
    {1256, "windows-1256", "1256 - العربية"},
    {1257, "windows-1257", "1257 - Baltų"},
    {1258, "windows-1258", "1258 - Tiếng Việt"},
    {65001, ENCODING_UTF8, "65001 - UTF-8"},
};

QByteArray CharsetInfoManager::getDefaultSystemEncoding()
{
#ifdef Q_OS_WIN
    unsigned acp = GetACP();
    for (const Win32AnsiCodePage &item : Win32AnsiCodePage::codePageList) {
        if (item.codePage == acp)
            return item.encoding;
    }
    return "unknown";
#else
    return ENCODING_UTF8;
#endif
}

QStringList CharsetInfoManager::languageNames()
{
    QSet<QString> languages;
    foreach (const PCharsetInfo& info, mCodePages) {
        languages.insert(info->language);
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
        if (info->language == languageName)
            result.append(info);
    }
    std::sort(result.begin(),result.end(),[](const PCharsetInfo& info1,const PCharsetInfo& info2){
        return (info1->name < info2->name);
    });
    return result;
}

QList<PCharsetInfo> CharsetInfoManager::findCharsetByLocale(const QString &localeName)
{
    QList<PCharsetInfo> result;
    foreach (const PCharsetInfo& info, mCodePages) {
        if (info->localeName == localeName)
            result.append(info);
    }
    return result;
}

QString CharsetInfoManager::findLanguageByCharsetName(const QString &encodingName)
{

    foreach (const PCharsetInfo& info, mCodePages) {
        if (QString::compare(info->name, encodingName, Qt::CaseInsensitive)==0)
            return info->language;
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
    mCodePages.append(std::make_shared<CharsetInfo>("tis-620",tr("Thai"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("shift_jis",tr("Japanese"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("gbk",tr("Chinese"),"zh_CN"));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-949",tr("Korean"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("big5",tr("Chinese"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("utf-16",tr("Unicode"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1250",tr("Central Europe"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1251",tr("Cyrillic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1252",tr("Western Europe"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1253",tr("Greek"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1254",tr("Turkish"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1255",tr("Hebrew"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1256",tr("Arabic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1257",tr("Baltic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("windows-1258",tr("Vietnamese"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("macintosh",tr("Cyrillic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("utf-32",tr("Unicode"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("koi8-r",tr("Cyrillic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("koi8-u",tr("Cyrillic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-1",tr("Western Europe"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-2",tr("Eastern Europe"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-3",tr("Turkish"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-4",tr("Baltic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-5",tr("Cyrillic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-6",tr("Arabic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-7",tr("Greek"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-8",tr("Hebrew"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-9",tr("Turkish"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-13",tr("Baltic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-14",tr("Celtic"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("iso-8859-15",tr("Western Europe"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("euc-jp",tr("Japanese"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("euc-kr",tr("Korean"),""));
    mCodePages.append(std::make_shared<CharsetInfo>("gb18030",tr("Chinese"),"zh_CN"));
    mCodePages.append(std::make_shared<CharsetInfo>("utf-8",tr("Unicode"),""));

}

CharsetInfo::CharsetInfo(const QByteArray &name, const QString &language,const QString& localeName)
{
    this->name = name;
    this->language = language;
    this->localeName = localeName;
}
