#include "toolchain_detect.h"
#include "../utils.h"
#include "../systemconsts.h"
#include "../compiler/compilerinfo.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVersionNumber>
#include <QRegularExpression>
#include <QProcessEnvironment>
#include <QUuid>

namespace ToolchainDetect {

// ---------------------------------------------------------------------------
// internal helpers
// ---------------------------------------------------------------------------

static QByteArray getCompilerOutput(const QString& binDir, const QString& binFile,
                                     const QStringList& arguments)
{
    QString cmd = getFilePath(binDir, binFile);
    QProcessEnvironment env;
    env.insert("LANGUAGE", "");
    env.insert("LC_ALL", "C");
    env.insert("PATH", binDir);
    auto [result, _, errorMessage] = runAndGetOutput(
        cmd, binDir, arguments, QByteArray(), false, false, env);
    if (!errorMessage.isEmpty())
        qWarning() << "[detect] getCompilerOutput error:" << cmd << errorMessage;
    return result.trimmed();
}

static void addExistingDirectory(QStringList& dirs, const QString& directory)
{
    if (!directoryExists(directory))
        return;
    QFileInfo dirInfo(directory);
    QString dirPath = dirInfo.absoluteFilePath();
    if (dirs.contains(dirPath))
        return;
    dirs.append(dirPath);
}

static void addExistingDirectory(QStringList& dirs, const QByteArray& directory)
{
    QString dir = QString::fromLocal8Bit(directory);
    if (!directoryExists(dir))
        dir = QString::fromUtf8(directory);
    addExistingDirectory(dirs, dir);
}

static bool isTarget64Bit(const QString& target)
{
    static const QStringList bits32 = {"i386", "i486", "i586", "i686", "arm", "mipsel", "mips"};
    for (const QString& s : bits32) {
        if (target.startsWith(s))
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// getCompilerIdentity
// ---------------------------------------------------------------------------

CompilerIdentity getCompilerIdentity(const QString& folder,
                                      const GnuToolchainName& c_prog)
{
    CompilerIdentity id;
    id.dumpMachine = QString(getCompilerOutput(folder, c_prog.fullName, {"-dumpmachine"})).trimmed();
    id.dumpVersion = QString(getCompilerOutput(folder, c_prog.fullName, {"-dumpversion"})).trimmed();
    QByteArray versionOut = getCompilerOutput(folder, c_prog.fullName, {"--version"});

    if (versionOut.contains("Apple clang version") || versionOut.contains("Apple LLVM")) {
        id.compilerType = CompilerType::AppleClang;
    } else if (versionOut.contains("clang version")) {
        id.compilerType = CompilerType::Clang;
    } else {
        id.compilerType = CompilerType::GCC;
    }
    return id;
}

// ---------------------------------------------------------------------------
// detectProperties (GCC / Clang / SDCC)
// ---------------------------------------------------------------------------

bool detectProperties(Toolchain& tc, const QString& binDir,
                       const GnuToolchainName& c_prog,
                       const CompilerIdentity& preIdentity)
{
    tc.dumpMachine = preIdentity.dumpMachine;
    tc.version = preIdentity.dumpVersion;
    tc.compilerType = preIdentity.compilerType;
    return detectProperties(tc, binDir, c_prog);
}

bool detectProperties(Toolchain& tc, const QString& binDir,
                       const GnuToolchainName& c_prog)
{
#ifdef ENABLE_SDCC
    if (c_prog.baseProgram == SDCC_PROGRAM) {
        // SDCC detection
        QStringList args;
        args << "-v";
        QByteArray output = getCompilerOutput(binDir, c_prog.fullName, args);
        if (!output.startsWith("SDCC")) {
            qWarning() << "[detect] SDCC output does not start with SDCC";
            return false;
        }
        int delimPos = 0;
        while (delimPos < output.length() && output[delimPos] >= (char)32)
            delimPos++;
        QString triplet = output.mid(0, delimPos);
        QRegularExpression re("\\s+(\\d+\\.\\d+\\.\\d+)\\s+");
        QRegularExpressionMatch match = re.match(triplet);
        if (match.hasMatch())
            tc.version = match.captured(1);
        if (tc.version.isEmpty())
            tc.name = "SDCC";
        else
            tc.name = "SDCC " + tc.version;
        tc.compilerType = CompilerType::SDCC;
        tc.dumpMachine = triplet;
        addExistingDirectory(tc.binDirs, binDir);
        return true;
    }
#endif

    // GCC/Clang detection
    if (tc.dumpMachine.isEmpty()) {
        QByteArray dm = getCompilerOutput(binDir, c_prog.fullName, {"-dumpmachine"});
        tc.dumpMachine = QString(dm).trimmed();
    }
    if (tc.dumpMachine.isEmpty()) {
        qWarning() << "[detect] empty dumpMachine for" << c_prog.fullName;
        return false;
    }
    if (tc.dumpMachine == "mingw32")
        tc.dumpMachine = "i386-pc-mingw32";

    tc.target = tc.dumpMachine.mid(0, tc.dumpMachine.indexOf('-'));

    bool outputFormatIsPe = false;
    if (tc.compilerType == CompilerType::Unknown) {
        QByteArray verboseOut = getCompilerOutput(binDir, c_prog.fullName, {"-v"});
        if (verboseOut.contains("Apple clang version") || verboseOut.contains("Apple LLVM")) {
            tc.compilerType = CompilerType::AppleClang;
        } else if (verboseOut.contains("clang version")) {
            tc.compilerType = CompilerType::Clang;
        } else {
            tc.compilerType = CompilerType::GCC;
        }
    }

    if (tc.compilerType == CompilerType::Clang || tc.compilerType == CompilerType::AppleClang) {
        if (tc.version.isEmpty()) {
            QByteArray ver = getCompilerOutput(binDir, c_prog.fullName, {"-dumpversion"}).trimmed();
            tc.version = QString(ver).trimmed();
        }

        // Parse Apple Clang version format
        if (tc.compilerType == CompilerType::AppleClang && tc.version.contains("Apple")) {
            tc.name = "Apple Clang";
        } else if (c_prog.hasTriplet || c_prog.hasVersion) {
            QStringList parts;
            parts << "Clang";
            if (c_prog.hasTriplet) parts << c_prog.tripletPrefix;
            if (c_prog.hasVersion) parts << c_prog.versionSuffix;
            tc.name = parts.join(' ');
        }
        // Full name detection
        QRegularExpression ntPosix("^(.*)-pc-windows-msys$");
        QRegularExpression mingwW64("^(.*)-w64-windows-gnu$");
        if (ntPosix.match(tc.dumpMachine).hasMatch()) {
            outputFormatIsPe = true;
            if (tc.name.isEmpty())
                tc.name = "MSYS2 Clang " + tc.version;
        } else if (mingwW64.match(tc.dumpMachine).hasMatch()) {
            outputFormatIsPe = true;
            if (tc.name.isEmpty())
                tc.name = "MinGW-w64 Clang " + tc.version;
        } else {
            if (tc.name.isEmpty())
                tc.name = "Clang " + tc.version;
        }
        // Apple Clang naming
        if (tc.compilerType == CompilerType::AppleClang && tc.name == "Clang " + tc.version)
            tc.name = "Apple Clang " + tc.version;
    } else {
        // GCC
        if (tc.version.isEmpty()) {
            tc.version = QString(getCompilerOutput(binDir, c_prog.fullName, {"-dumpversion"}).trimmed()).trimmed();
            if (!tc.version.isEmpty() && !tc.version.contains('.')) {
                QByteArray full = getCompilerOutput(binDir, c_prog.fullName, {"-dumpfullversion"}).trimmed();
                if (!full.isEmpty())
                    tc.version = QString(full).trimmed();
            }
        }
        if (c_prog.hasTriplet || c_prog.hasVersion) {
            QStringList parts;
            parts << "GCC";
            if (c_prog.hasTriplet) parts << c_prog.tripletPrefix;
            if (c_prog.hasVersion) parts << c_prog.versionSuffix;
            tc.name = parts.join(' ');
        }
        QRegularExpression ntPosix("^(.*)-(.*)-(msys|cygwin)$");
        QRegularExpression mingwW64("^(.*)-w64-mingw32$");
        QRegularExpression mingwOrg("^(.*)-(.*)-mingw32$");
        if (auto m = ntPosix.match(tc.dumpMachine); m.hasMatch()) {
            outputFormatIsPe = true;
            if (tc.name.isEmpty())
                tc.name = (m.captured(3) == "msys") ? "MSYS2 GCC " + tc.version : "Cygwin GCC " + tc.version;
        } else if (mingwW64.match(tc.dumpMachine).hasMatch()) {
            outputFormatIsPe = true;
            if (tc.name.isEmpty())
                tc.name = "MinGW-w64 GCC " + tc.version;
        } else if (mingwOrg.match(tc.dumpMachine).hasMatch()) {
            outputFormatIsPe = true;
            if (tc.name.isEmpty())
                tc.name = "MinGW.org GCC " + tc.version;
        } else {
            if (tc.name.isEmpty())
                tc.name = "GCC " + tc.version;
        }
    }

    QDir tmpDir(binDir);
    tmpDir.cdUp();
    addExistingDirectory(tc.binDirs, getFilePath(tmpDir.path(), "bin"));
    return true;
}

// ---------------------------------------------------------------------------
// detectExecutables
// ---------------------------------------------------------------------------

void detectExecutables(Toolchain& tc, const GnuToolchainName& c_prog)
{
    QString prefix = c_prog.hasTriplet ? c_prog.tripletPrefix + "-" : QString();
    QString verSuffix = c_prog.hasVersion ? "-" + c_prog.versionSuffix : QString();

    auto findInBins = [&](const QString& name) -> QString {
        for (const QString& d : tc.binDirs) {
            QString full = getFilePath(d, name);
            if (QFileInfo::exists(full) && QFileInfo(full).isExecutable())
                return full;
        }
        return {};
    };

    if (tc.compilerType == CompilerType::Clang || tc.compilerType == CompilerType::AppleClang) {
        tc.ccompiler = findInBins(prefix + CLANG_PROGRAM + verSuffix);
        if (tc.ccompiler.isEmpty())
            tc.ccompiler = findInBins(prefix + GCC_PROGRAM + verSuffix);
        tc.cppCompiler = findInBins(prefix + CLANG_CPP_PROGRAM + verSuffix);
        if (tc.cppCompiler.isEmpty())
            tc.cppCompiler = findInBins(prefix + GPP_PROGRAM + verSuffix);
        tc.debugger = findInBins(prefix + GDB_PROGRAM + verSuffix);
        if (tc.debugger.isEmpty()) {
            tc.debugger = findInBins(prefix + LLDB_MI_PROGRAM + verSuffix);
            tc.debugServer = findInBins(LLDB_SERVER_PROGRAM);
        } else {
            tc.debugServer = findInBins(GDB_SERVER_PROGRAM);
        }
#ifdef ENABLE_SDCC
    } else if (tc.compilerType == CompilerType::SDCC) {
        tc.ccompiler = findInBins(SDCC_PROGRAM);
#endif
    } else {
        tc.ccompiler = findInBins(prefix + GCC_PROGRAM + verSuffix);
        tc.cppCompiler = findInBins(prefix + GPP_PROGRAM + verSuffix);
        tc.debugger = findInBins(prefix + GDB_PROGRAM + verSuffix);
        tc.debugServer = findInBins(GDB_SERVER_PROGRAM);
    }
    tc.make = findInBins(MAKE_PROGRAM);
#ifdef Q_OS_WIN
    tc.resourceCompiler = findInBins(WINDRES_PROGRAM);
#endif
}

// ---------------------------------------------------------------------------
// detectDirectories
// ---------------------------------------------------------------------------

void detectDirectories(Toolchain& tc, const QString& binDir,
                        const GnuToolchainName& c_prog)
{
#ifdef ENABLE_SDCC
    if (tc.compilerType == CompilerType::SDCC) {
        // SDCC directory detection
        QStringList args;
        args << "--print-search-dirs";
        QString key = SDCC_CMD_OPT_PROCESSOR;
        PCompilerOption pOption = CompilerInfoManager::getCompilerOption(tc.compilerType, key);
        if (pOption) {
            QString val = tc.compilerOptions.value(key);
            if (!val.isEmpty())
                args << pOption->setting + val;
        }
        QByteArray output = getCompilerOutput(binDir, SDCC_PROGRAM, args);

        auto parseSection = [&](const QByteArray& start, const QByteArray& end) {
            int p1 = output.indexOf(start);
            int p2 = output.indexOf(end);
            if (p1 > 0 && p2 > p1) {
                p1 += start.length();
                QList<QByteArray> lines = output.mid(p1, p2 - p1).split('\n');
                for (QByteArray& line : lines) {
                    QByteArray trimmed = line.trimmed();
                    if (!trimmed.isEmpty())
                        addExistingDirectory(tc.binDirs, trimmed);
                }
            }
        };

        parseSection("programs:", "datadir:");
        parseSection("includedir:", "libdir:");
        parseSection("libdir:", "libpath:");
        return;
    }
#endif

    // GCC/Clang directory detection
    QString progName = c_prog.fullName;
    if (progName.isEmpty())
        progName = extractFileName(tc.ccompiler);
    if (progName.isEmpty()) {
        progName = (tc.compilerType == CompilerType::Clang || tc.compilerType == CompilerType::AppleClang)
            ? CLANG_PROGRAM : GCC_PROGRAM;
    }

    // C include dirs
    {
        QStringList args;
        args << "-xc" << "-v" << "-E" << NULL_FILE;
        QByteArray output = getCompilerOutput(binDir, progName, args);
        int p1 = output.indexOf("#include <...> search starts here:");
        int p2 = output.indexOf("End of search list.");
        if (p1 > 0 && p2 > 0) {
            p1 += QByteArray("#include <...> search starts here:").length();
            QList<QByteArray> lines = output.mid(p1, p2 - p1).split('\n');
            for (QByteArray& line : lines) {
                QByteArray trimmed = line.trimmed();
                if (!trimmed.isEmpty())
                    addExistingDirectory(tc.cIncludeDirs, trimmed);
            }
        }
    }

    // C++ include dirs
    {
        QStringList args;
        args << "-xc++" << "-E" << "-v" << NULL_FILE;
        QByteArray output = getCompilerOutput(binDir, progName, args);
        int p1 = output.indexOf("#include <...> search starts here:");
        int p2 = output.indexOf("End of search list.");
        if (p1 > 0 && p2 > 0) {
            p1 += QByteArray("#include <...> search starts here:").length();
            QList<QByteArray> lines = output.mid(p1, p2 - p1).split('\n');
            for (QByteArray& line : lines) {
                QByteArray trimmed = line.trimmed();
                if (!trimmed.isEmpty())
                    addExistingDirectory(tc.cppIncludeDirs, trimmed);
            }
        }
    }

    // Program & lib dirs via -print-search-dirs
    {
        QStringList args;
        args << "-print-search-dirs" << NULL_FILE;
        QByteArray output = getCompilerOutput(binDir, progName, args);

        // programs
        QByteArray targetStr = "programs: =";
        int p1 = output.indexOf(targetStr);
        if (p1 >= 0) {
            p1 += targetStr.length();
            int p2 = p1;
            while (p2 < output.length() && output[p2] != '\n') p2++;
            QList<QByteArray> lines = output.mid(p1, p2 - p1).split(';');
            for (QByteArray& line : lines) {
                QByteArray trimmed = line.trimmed();
                if (!trimmed.isEmpty())
                    addExistingDirectory(tc.binDirs, trimmed);
            }
        }

        // libraries
        targetStr = "libraries: =";
        p1 = output.indexOf(targetStr);
        if (p1 >= 0) {
            p1 += targetStr.length();
            int p2 = p1;
            while (p2 < output.length() && output[p2] != '\n') p2++;
            QList<QByteArray> lines = output.mid(p1, p2 - p1).split(';');
            for (QByteArray& line : lines) {
                QByteArray trimmed = line.trimmed();
                if (!trimmed.isEmpty())
                    addExistingDirectory(tc.libDirs, trimmed);
            }
        }
    }
}

QList<Toolchain> discoverClangTriplets(
    const QString& folder,
    const GnuToolchainName& c_prog,
    const QSet<QString>& existingKeys,
    const CompilerIdentity& preIdentity)
{
    QList<Toolchain> result;

    QString versionPart;
    if (c_prog.hasVersion)
        versionPart = c_prog.versionSuffix;

    QDir binDir(folder);
    QDir scanDir = binDir;
    if (!scanDir.cdUp()) {
        qWarning() << "[clang triplets] cannot cd up from:" << binDir.absolutePath();
        return result;
    }

    qDebug() << "[clang triplets] scanning:" << scanDir.absolutePath()
             << "existing:" << existingKeys;

    for (const QString& entry : scanDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (!GnuToolchainName::looksLikeTripletDirectory(entry))
            continue;

        QString variantKey = entry + "-" + versionPart;
        if (existingKeys.contains(variantKey)) {
            qDebug() << "[clang triplets]   -> already exists, skipping:" << entry;
            continue;
        }

        qDebug() << "[clang triplets]   found triplet dir:" << entry;

        Toolchain tc;
        tc.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (!detectProperties(tc, folder, c_prog, preIdentity)) {
            qWarning() << "[clang triplets]   -> detectProperties failed for" << entry;
            continue;
        }

        detectExecutables(tc, c_prog);
        tc.clangTarget = entry;

        // Name: "Clang 18.1.8 x86_64-linux-gnu"
        QString baseName = tc.name;
        bool useFilenameNaming = c_prog.hasTriplet || c_prog.hasVersion;
        if (useFilenameNaming) {
            tc.name = baseName + " " + entry;
        } else {
            QString platformLabel = isTarget64Bit(tc.target)
                ? QStringLiteral("64-bit") : QStringLiteral("32-bit");
            tc.name = baseName + " " + entry + " " + platformLabel;
        }

        // Detect directories (may include target-specific paths)
        detectDirectories(tc, folder, c_prog);

        result.append(tc);
    }

    return result;
}

} // namespace ToolchainDetect
