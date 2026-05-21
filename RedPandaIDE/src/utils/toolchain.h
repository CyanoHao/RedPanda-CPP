/*
 * Copyright (C) 2020-2026 Roy Qu (royqh1979@gmail.com)
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

#ifndef UTILS_TOOLCHAIN_H
#define UTILS_TOOLCHAIN_H

#include <QString>

struct GnuToolchainName
{
    bool isValid = false;
    bool hasTriplet = false;
    bool hasVersion = false;
    QString fullName;
    QString tripletPrefix;
    QString baseProgram;
    QString versionSuffix;

    static GnuToolchainName parse(const QString &exeName);
    static bool isKnownCCompilerBaseName(const QString &name);
    static QString canonicalBaseName(const QString &exeName);
    static bool startsWithKnownArch(const QString &prefix);
    static bool isVersionString(const QString &s);
    static int generalPurposeRegisterWidth(const QString &arch);
    static QString stripExeSuffix(const QString &name);
    static bool looksLikeTripletDirectory(const QString &dirName);
};

#endif
