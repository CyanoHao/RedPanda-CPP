#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <memory>
#include "../compiler/compilerinfo.h"
#include "compilationstage.h"

struct CompilerIdentity {
    QString dumpMachine;
    QString dumpVersion;
    CompilerType compilerType = CompilerType::Unknown;
    bool isValid() const { return !dumpMachine.isEmpty(); }
};

enum class LinkModel {
    Dynamic,
    Static,
    PIE,
    StaticPIE
};

class Toolchain {
public:
    QString     id;                  // UUID
    QString     name;                // e.g. "MinGW-w64 GCC 14.2.0"
    CompilerType compilerType = CompilerType::Unknown;

    // executable paths
    QString     ccompiler;
    QString     cppCompiler;
    QString     make;
    QString     debugger;
    QString     resourceCompiler;   // windres / rc.exe
    QString     debugServer;

    // directories
    QStringList binDirs;
    QStringList cIncludeDirs;
    QStringList cppIncludeDirs;
    QStringList libDirs;

    // compiler identity
    QString     dumpMachine;
    QString     version;
    QString     target;             // e.g. "x86_64"

    // Clang-specific
    QString     clangTarget;

    // toolchain-level compiler options (see design doc appendix A)
    QMap<QString,QString> compilerOptions;

    // misc
    bool        forceEnglishOutput = false;
    QString     preprocessingSuffix = ".i";
    QString     compilationProperSuffix = ".s";
    QString     assemblingSuffix = ".o";
    QString     executableSuffix;          // "" (linux) / ".exe" (windows)

    LinkModel   defaultLinkModel = LinkModel::Dynamic;

    // methods
    bool canCompileC() const;
    bool canCompileCpp() const;
    bool canMake() const;
    bool canDebug() const;
    bool isValid() const;

    // A group: pure data methods (no cache, no subprocess)
    QString findProgramInDirs(const QString& name) const;
    QString getOutputFilename(const QString& sourceFilename,
                              CompilationStage stage = CompilationStage::GenerateExecutable) const;
    static bool isOutputExecutable(CompilationStage stage);
    QStringList findErrors() const;

    // B group: cached methods (cache, no subprocess, Windows only)
#ifdef Q_OS_WINDOWS
    bool isCompilerUsingUTF8() const;
    bool isDebuggerUsingUTF8() const;
#else
    constexpr bool isCompilerUsingUTF8() const { return true; }
    constexpr bool isDebuggerUsingUTF8() const { return true; }
#endif

    // C group: lazy detection methods (cache + subprocess, first access triggers)
    QStringList defaultCIncludeDirs() const;
    QStringList defaultCppIncludeDirs() const;
    QStringList defaultLibDirs() const;
    bool supportConvertingCharset();
    bool supportNLS();

    // D group: composite dependency methods (depend on compile options)
    QStringList defines(bool isCpp, const QMap<QString,QString>& mergedOptions) const;

private:
    // Lazy directory detection cache
    mutable bool mDirectoriesDetected = false;
    void ensureDirectoriesDetected() const;

    // GCC configure string cache (for supportConvertingCharset and supportNLS)
    mutable QString mGccConfigureString;
    mutable bool mGccConfigureInitialized = false;
    void ensureGccConfigure() const;

    // Windows UTF-8 detection cache
#ifdef Q_OS_WINDOWS
    mutable bool mCompilerIsUtf8 = false;
    mutable bool mCompilerIsUtf8Initialized = false;
    mutable bool mDebuggerIsUtf8 = false;
    mutable bool mDebuggerIsUtf8Initialized = false;
#endif
};

using PToolchain = std::shared_ptr<Toolchain>;

#endif // TOOLCHAIN_H
