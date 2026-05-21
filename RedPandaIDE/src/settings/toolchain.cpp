#include "toolchain.h"
#include "toolchain_detect.h"
#include "../utils/toolchain.h"         // GnuToolchainName
#include "../utils.h"                   // fileExists, changeFileExt, getAbsoluteFilePath, NULL_FILE
#include "../systemconsts.h"            // Program name constants
#include <QFileInfo>
#include <QDir>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QDebug>

#ifdef Q_OS_WINDOWS
#include "../utils/portableexecutable.h"
#endif

bool Toolchain::canCompileC() const
{
    return !ccompiler.isEmpty() && QFileInfo::exists(ccompiler);
}

bool Toolchain::canCompileCpp() const
{
    return !cppCompiler.isEmpty() && QFileInfo::exists(cppCompiler);
}

bool Toolchain::canMake() const
{
    return !make.isEmpty();
}

bool Toolchain::canDebug() const
{
    return !debugger.isEmpty() && QFileInfo::exists(debugger);
}

bool Toolchain::isValid() const
{
    return canCompileC() || canCompileCpp();
}

// ---------------------------------------------------------------------------
// internal helper: getCompilerOutput
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
        qDebug() << "[Toolchain] getCompilerOutput error:" << cmd << errorMessage;
    return result.trimmed();
}

// ---------------------------------------------------------------------------
// A group: pure data methods (no cache, no subprocess)
// ---------------------------------------------------------------------------

QString Toolchain::findProgramInDirs(const QString& name) const
{
    for (const QString& dir : binDirs) {
        QFileInfo f(getAbsoluteFilePath(dir, name));
        if (f.exists() && f.isExecutable())
            return f.absoluteFilePath();
    }
    return {};
}

QString Toolchain::getOutputFilename(const QString& sourceFilename, CompilationStage stage) const
{
    switch (stage) {
    case CompilationStage::PreprocessingOnly:
        return changeFileExt(sourceFilename, preprocessingSuffix);
    case CompilationStage::CompilationProperOnly:
        return changeFileExt(sourceFilename, compilationProperSuffix);
    case CompilationStage::GenerateGimple:
        return changeFileExt(sourceFilename, "gimple");
    case CompilationStage::AssemblingOnly:
        return changeFileExt(sourceFilename, assemblingSuffix);
    case CompilationStage::GenerateExecutable:
        return changeFileExt(sourceFilename, executableSuffix);
    }
    return changeFileExt(sourceFilename, DEFAULT_EXECUTABLE_SUFFIX);
}

bool Toolchain::isOutputExecutable(CompilationStage stage)
{
    return stage == CompilationStage::GenerateExecutable;
}

QStringList Toolchain::findErrors() const
{
    QStringList errors;
    if (!ccompiler.isEmpty() && !fileExists(ccompiler))
        errors.append(QObject::tr("C Compiler \"%1\" is missing!").arg(ccompiler));
    if (!cppCompiler.isEmpty() && !fileExists(cppCompiler))
        errors.append(QObject::tr("C++ Compiler \"%1\" is missing!").arg(cppCompiler));
    if (!debugger.isEmpty() && !fileExists(debugger))
        errors.append(QObject::tr("Debugger \"%1\" is missing!").arg(debugger));
    if (!make.isEmpty() && !fileExists(make))
        errors.append(QObject::tr("Make program \"%1\" is missing!").arg(make));
    return errors;
}

// ---------------------------------------------------------------------------
// B group: cached methods (cache, no subprocess, Windows only)
// ---------------------------------------------------------------------------

#ifdef Q_OS_WINDOWS
bool Toolchain::isCompilerUsingUTF8() const
{
    if (!mCompilerIsUtf8Initialized) {
        mCompilerIsUtf8 = PortableExecutable(ccompiler).isUtf8();
        mCompilerIsUtf8Initialized = true;
    }
    return mCompilerIsUtf8;
}

bool Toolchain::isDebuggerUsingUTF8() const
{
    if (!mDebuggerIsUtf8Initialized) {
        mDebuggerIsUtf8 = PortableExecutable(debugger).isUtf8();
        mDebuggerIsUtf8Initialized = true;
    }
    return mDebuggerIsUtf8;
}
#endif

// ---------------------------------------------------------------------------
// C group: lazy detection methods (cache + subprocess, first access triggers)
// ---------------------------------------------------------------------------

void Toolchain::ensureDirectoriesDetected() const
{
    if (mDirectoriesDetected)
        return;
    mDirectoriesDetected = true;

    if (ccompiler.isEmpty())
        return;

    QString binDir = QFileInfo(ccompiler).dir().path();
    GnuToolchainName emptyProg;
    ToolchainDetect::detectDirectories(const_cast<Toolchain&>(*this), binDir, emptyProg);
}

QStringList Toolchain::defaultCIncludeDirs() const
{
    ensureDirectoriesDetected();
    return cIncludeDirs;
}

QStringList Toolchain::defaultCppIncludeDirs() const
{
    ensureDirectoriesDetected();
    return cppIncludeDirs;
}

QStringList Toolchain::defaultLibDirs() const
{
    ensureDirectoriesDetected();
    return libDirs;
}

void Toolchain::ensureGccConfigure() const
{
    if (mGccConfigureInitialized)
        return;
    mGccConfigureInitialized = true;

    if (compilerType != CompilerType::GCC)
        return;

    if (ccompiler.isEmpty())
        return;

    QString binDir = QFileInfo(ccompiler).dir().path();
    QString progName = extractFileName(ccompiler);
    if (progName.isEmpty())
        progName = GCC_PROGRAM;

    QByteArray verboseOut = getCompilerOutput(binDir, progName, {"-v"});
    QByteArray targetStr = "Configured with: ";
    int configurationPos = verboseOut.indexOf(targetStr);
    if (configurationPos < 0)
        return;
    int endPos = verboseOut.indexOf('\n', configurationPos);
    if (endPos < 0)
        return;
    mGccConfigureString = QString::fromUtf8(
        verboseOut.mid(configurationPos + targetStr.size(), endPos - configurationPos - targetStr.size())
    );
}

bool Toolchain::supportConvertingCharset()
{
#ifdef Q_OS_WINDOWS
    ensureGccConfigure();
    return compilerType == CompilerType::GCC
        && mGccConfigureString.contains("--with-libiconv")
        && !mGccConfigureString.contains("--with-libiconv=no");
#else
    return compilerType == CompilerType::GCC;
#endif
}

bool Toolchain::supportNLS()
{
    ensureGccConfigure();
    return compilerType == CompilerType::GCC
        && mGccConfigureString.contains("--enable-nls")
        && !mGccConfigureString.contains("--disable-nls");
}

// ---------------------------------------------------------------------------
// D group: composite dependency methods (depend on compile options)
// ---------------------------------------------------------------------------

QStringList Toolchain::defines(bool isCpp, const QMap<QString, QString>& mergedOptions) const
{
    QStringList arguments;
    arguments.append("-dM");
    arguments.append("-E");
    arguments.append("-x");

    QString key;
#ifdef ENABLE_SDCC
    if (compilerType == CompilerType::SDCC) {
        arguments.append("c");
        arguments.append("-V");
        key = SDCC_CMD_OPT_PROCESSOR;
        PCompilerOption pOption = CompilerInfoManager::getCompilerOption(compilerType, key);
        if (pOption && !mergedOptions.value(key).isEmpty())
            arguments.append(pOption->setting + mergedOptions.value(key));
        key = SDCC_CMD_OPT_STD;
        pOption = CompilerInfoManager::getCompilerOption(compilerType, key);
        if (pOption && !mergedOptions.value(key).isEmpty())
            arguments.append(pOption->setting + mergedOptions.value(key));
    } else {
#endif
        if (isCpp) {
            arguments.append("c++");
            key = CC_CMD_OPT_STD;
        } else {
            arguments.append("c");
            key = C_CMD_OPT_STD;
        }
        PCompilerOption pOption = CompilerInfoManager::getCompilerOption(compilerType, key);
        if (pOption && !mergedOptions.value(key).isEmpty())
            arguments.append(pOption->setting + mergedOptions.value(key));
        pOption = CompilerInfoManager::getCompilerOption(compilerType, CC_CMD_OPT_ENABLE_GCC_IMPORT_STD);
        if (pOption && mergedOptions.contains(CC_CMD_OPT_ENABLE_GCC_IMPORT_STD))
            arguments.append(pOption->setting);
        pOption = CompilerInfoManager::getCompilerOption(compilerType, CC_CMD_OPT_DEBUG_INFO);
        if (pOption && mergedOptions.contains(CC_CMD_OPT_DEBUG_INFO))
            arguments.append(pOption->setting);
#ifdef ENABLE_SDCC
    }
#endif

    if (arguments.contains("-g3"))
        arguments.append("-D_DEBUG");
    arguments.append(NULL_FILE);

    QFileInfo ccompilerInfo(ccompiler);
    QByteArray output = getCompilerOutput(ccompilerInfo.dir().path(), ccompilerInfo.fileName(), arguments);

    QStringList result;
#ifdef ENABLE_SDCC
    if (compilerType == CompilerType::SDCC) {
        QList<QByteArray> lines = output.split('\n');
        QByteArray currentLine;
        for (QByteArray& line : lines) {
            QByteArray trimmedLine = line.trimmed();
            if (trimmedLine.startsWith("+")) {
                currentLine = line;
                break;
            }
        }
        lines = currentLine.split(' ');
        for (QByteArray& line : lines) {
            QByteArray trimmedLine = line.trimmed();
            if (trimmedLine.startsWith("-D")) {
                trimmedLine = trimmedLine.mid(2);
                if (trimmedLine.contains("=")) {
                    QList<QByteArray> items = trimmedLine.split('=');
                    result.append(QString("#define %1 %2").arg(QString(items[0]), QString(items[1])));
                } else {
                    result.append("#define " + trimmedLine);
                }
            }
        }
    } else {
#else
    {
#endif
        QList<QByteArray> lines = output.split('\n');
        for (QByteArray& line : lines) {
            QByteArray trimmedLine = line.trimmed();
            if (!trimmedLine.isEmpty())
                result.append(QString::fromUtf8(trimmedLine));
        }
    }
    return result;
}

