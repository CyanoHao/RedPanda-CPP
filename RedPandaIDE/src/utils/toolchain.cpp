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

#include "toolchain.h"

#include "llvm-triple.h"

#include <QRegularExpression>

GnuToolchainName GnuToolchainName::parse(const QString &exeName)
{
    GnuToolchainName result;
    QString name = stripExeSuffix(exeName);

    static const QStringList baseNames = {
        "clang++", "g++",
        "clang", "gcc", "sdcc"
    };

    for (const QString &base : baseNames) {
        int pos = name.indexOf(base);
        if (pos < 0)
            continue;

        QString prefix = name.left(pos);
        QString suffix = name.mid(pos + base.length());

        if (!suffix.isEmpty()) {
            if (!suffix.startsWith('-'))
                continue;
            QString verStr = suffix.mid(1);
            if (!isVersionString(verStr))
                continue;
            result.versionSuffix = verStr;
            result.hasVersion = true;
        }

        if (!prefix.isEmpty()) {
            if (!prefix.endsWith('-'))
                continue;
            if (prefix.count('-') < 3)
                continue;
            QString tripletCandidate = prefix.left(prefix.length() - 1);
            if (!startsWithKnownArch(tripletCandidate))
                continue;
            result.tripletPrefix = tripletCandidate;
            result.hasTriplet = true;
        }

        result.baseProgram = base;
        result.fullName = exeName;
        result.isValid = true;
        return result;
    }

    return result;
}

bool GnuToolchainName::isKnownCCompilerBaseName(const QString &name)
{
    return name == "gcc" || name == "clang" || name == "sdcc";
}

QString GnuToolchainName::canonicalBaseName(const QString &exeName)
{
    GnuToolchainName pr = parse(exeName);
    if (pr.isValid)
        return pr.baseProgram;
    return QString();
}

bool GnuToolchainName::startsWithKnownArch(const QString &s)
{
    int dashPos = s.indexOf('-');
    QString arch = (dashPos > 0) ? s.left(dashPos) : s;
    return llvm::parseArch(arch) != llvm::Triple::UnknownArch;
}

bool GnuToolchainName::isVersionString(const QString &s)
{
    if (s.isEmpty())
        return false;
    static const QRegularExpression re("^\\d+(\\.\\d+)*$");
    return re.match(s).hasMatch();
}

int GnuToolchainName::generalPurposeRegisterWidth(const QString &arch)
{
    using AT = llvm::Triple::ArchType;
    switch (llvm::parseArch(arch)) {
    case AT::x86_64:
    case AT::aarch64:
    case AT::aarch64_be:
    case AT::ppc64:
    case AT::ppc64le:
    case AT::mips64:
    case AT::mips64el:
    case AT::riscv64:
    case AT::riscv64be:
    case AT::sparcv9:
    case AT::systemz:
    case AT::wasm64:
    case AT::loongarch64:
    case AT::nvptx64:
    case AT::amdil64:
    case AT::hsail64:
    case AT::spir64:
    case AT::spirv64:
    case AT::renderscript64:
        return 64;
    case AT::UnknownArch:
        return 0;
    default:
        return 32;
    }
}

QString GnuToolchainName::stripExeSuffix(const QString &name)
{
#ifdef Q_OS_WIN
    if (name.endsWith(".exe", Qt::CaseInsensitive))
        return name.left(name.length() - 4);
#endif
    return name;
}

bool GnuToolchainName::looksLikeTripletDirectory(const QString &dirName)
{
    int firstDash = dirName.indexOf('-');
    if (firstDash <= 0)
        return false;
    QString arch = dirName.left(firstDash);
    if (llvm::parseArch(arch) == llvm::Triple::UnknownArch)
        return false;
    return dirName.count('-') >= 2;
}
