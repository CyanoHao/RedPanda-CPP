#include "toolchainmanager.h"
#include "toolchain_json.h"
#include "toolchain_detect.h"
#include "../utils.h"
#include "../systemconsts.h"
#include "../settings.h"
#include <QDebug>
#include <QDir>
#include <QUuid>
#include <QProcessEnvironment>
#include <QVersionNumber>
#include <QDateTime>
#include <algorithm>

ToolchainManager::ToolchainManager()
    : mToolchains(), mDefaultIndex(-1), mDefaultIndexTimestamp(0)
{
}

QList<Toolchain> ToolchainManager::toolchains() const
{
    return mToolchains;
}

PToolchain ToolchainManager::findById(const QString& id) const
{
    for (const auto& tc : mToolchains) {
        if (tc.id == id)
            return std::make_shared<Toolchain>(tc);
    }
    return nullptr;
}

PToolchain ToolchainManager::defaultToolchain() const
{
    if (mDefaultIndex >= 0 && mDefaultIndex < mToolchains.size())
        return std::make_shared<Toolchain>(mToolchains[mDefaultIndex]);
    if (!mToolchains.isEmpty())
        return std::make_shared<Toolchain>(mToolchains.first());
    return nullptr;
}

int ToolchainManager::defaultIndex() const
{
    if (mDefaultIndex >= 0 && mDefaultIndex < mToolchains.size())
        return mDefaultIndex;
    if (!mToolchains.isEmpty())
        return 0;
    return -1;
}

qint64 ToolchainManager::defaultIndexTimestamp() const
{
    return mDefaultIndexTimestamp;
}

void ToolchainManager::setDefaultIndex(int index)
{
    if (index >= 0 && index < mToolchains.size()) {
        mDefaultIndex = index;
        mDefaultIndexTimestamp = QDateTime::currentMSecsSinceEpoch();
    }
}

void ToolchainManager::setDefaultToolchain(const QString& id)
{
    for (int i = 0; i < mToolchains.size(); ++i) {
        if (mToolchains[i].id == id) {
            mDefaultIndex = i;
            return;
        }
    }
}

void ToolchainManager::addToolchain(const Toolchain& tc)
{
    // check for duplicate by id
    for (const auto& existing : mToolchains) {
        if (existing.id == tc.id) {
            return; // already exists
        }
    }
    mToolchains.append(tc);
}

void ToolchainManager::removeToolchain(const QString& id)
{
    for (int i = 0; i < mToolchains.size(); ++i) {
        if (mToolchains[i].id == id) {
            mToolchains.removeAt(i);
            if (mDefaultIndex >= mToolchains.size())
                mDefaultIndex = mToolchains.size() - 1;
            if (mDefaultIndex > i)
                mDefaultIndex--;
            return;
        }
    }
}

void ToolchainManager::updateToolchain(const QString& id, const Toolchain& tc)
{
    for (int i = 0; i < mToolchains.size(); ++i) {
        if (mToolchains[i].id == id) {
            mToolchains[i] = tc;
            mToolchains[i].id = id; // keep original id
            return;
        }
    }
}

void ToolchainManager::load(const QString& filename)
{
    mToolchains = ToolchainJson::load(filename);
    mDefaultIndex = mToolchains.isEmpty() ? -1 : 0;
}

void ToolchainManager::save(const QString& filename) const
{
    QList<Toolchain> list = mToolchains;
    ToolchainJson::save(filename, list);
}

int ToolchainManager::size() const
{
    return mToolchains.size();
}

void ToolchainManager::discover()
{
    qDebug() << "[toolchain discover] === discover started ===";

    // Clear existing list
    mToolchains.clear();
    mDefaultIndex = -1;

    QSet<QString> searched;

    // Scan bundled toolchain directories
    QDir libexecDir(pSettings->dirs().appLibexecDir());
    static const QStringList compilerDirPatterns = {
        "clang*", "gcc*", "llvm*", "mingw*", "ucrt*", "w64devkit*",
    };
    QStringList libexecBins;
    for (const QString& entry : libexecDir.entryList(compilerDirPatterns, QDir::Dirs)) {
        QString binPath = libexecDir.absoluteFilePath(entry + "/bin");
        if (QDir(binPath).exists())
            libexecBins.append(binPath);
    }

    // Build search path: bundled dirs + system PATH
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList pathList = env.value("PATH").split(PATH_SEPARATOR);
    pathList = libexecBins + pathList;

    // Search in reverse order (bundled toolchains first)
    for (int i = pathList.count() - 1; i >= 0; i--) {
        QString folder = QDir(pathList[i]).absolutePath();
        QString canonicalFolder = QDir(pathList[i]).canonicalPath();
        if (canonicalFolder.isEmpty())
            continue;
        if (searched.contains(canonicalFolder))
            continue;
        searched.insert(canonicalFolder);
        discoverFromFolder(folder);
    }

    if (mDefaultIndex < 0 && !mToolchains.isEmpty())
        mDefaultIndex = 0;

    qDebug() << "[toolchain discover] === finished:" << mToolchains.size() << "toolchain(s) ===";
}

bool ToolchainManager::discoverFromFolder(const QString& folder)
{
    if (!directoryExists(folder))
        return false;

    qDebug() << "[toolchain discover] scanning:" << folder;

    QDir dir(folder);
    QStringList entries = dir.entryList(QDir::Files);

    struct Candidate {
        GnuToolchainName prog;
        CompilerIdentity identity;
        int nameLength;
    };

    auto identityKey = [](const CompilerIdentity& id) {
        return id.dumpMachine + QChar('\0') + id.dumpVersion
             + QChar('\0') + QString::number(static_cast<int>(id.compilerType));
    };

    QMap<QString, Candidate> bestForIdentity;
    QList<GnuToolchainName> sdccCandidates;

    for (const QString& entry : entries) {
        QFileInfo fi(getAbsoluteFilePath(folder, entry));
        if (!fi.isExecutable())
            continue;

        auto pr = GnuToolchainName::parse(entry);
        if (!pr.isValid)
            continue;
        if (!GnuToolchainName::isKnownCCompilerBaseName(pr.baseProgram))
            continue;

        if (pr.baseProgram == SDCC_PROGRAM) {
            sdccCandidates.append(pr);
            continue;
        }

        CompilerIdentity id = ToolchainDetect::getCompilerIdentity(folder, pr);
        if (!id.isValid()) {
            qDebug() << "[toolchain discover]   -> invalid identity for" << entry;
            continue;
        }

        QString key = identityKey(id);
        auto it = bestForIdentity.find(key);
        if (it == bestForIdentity.end()) {
            bestForIdentity[key] = Candidate{pr, id, (int)entry.length()};
        } else {
            Candidate& existing = it.value();
            bool newIsPreferred = false;
            if (id.compilerType == CompilerType::Clang
                && existing.prog.baseProgram != CLANG_PROGRAM
                && pr.baseProgram == CLANG_PROGRAM)
                newIsPreferred = true;
            else if (id.compilerType == CompilerType::GCC
                     && existing.prog.baseProgram != GCC_PROGRAM
                     && pr.baseProgram == GCC_PROGRAM)
                newIsPreferred = true;
            else if (entry.length() < existing.nameLength)
                newIsPreferred = true;
            if (newIsPreferred)
                existing = Candidate{pr, id, (int)entry.length()};
        }
    }

    // Collect selected compilers
    QList<GnuToolchainName> selectedCompilers;
    QMap<QString, CompilerIdentity> selectedIdentities;
    for (auto it = bestForIdentity.begin(); it != bestForIdentity.end(); ++it) {
        selectedCompilers.append(it.value().prog);
        selectedIdentities[it.value().prog.fullName] = it.value().identity;
    }

    // Sort (8-tier system)
    auto sortOrder = [](const GnuToolchainName& p) -> int {
        if (p.baseProgram == GCC_PROGRAM) {
            if (!p.hasTriplet) return p.hasVersion ? 2 : 0;
            return p.hasVersion ? 6 : 4;
        }
        if (p.baseProgram == CLANG_PROGRAM) {
            if (!p.hasTriplet) return p.hasVersion ? 3 : 1;
            return p.hasVersion ? 7 : 5;
        }
        return 8;
    };

    std::sort(selectedCompilers.begin(), selectedCompilers.end(),
        [&](const GnuToolchainName& a, const GnuToolchainName& b) {
            int oa = sortOrder(a), ob = sortOrder(b);
            if (oa != ob) return oa < ob;
            if (oa == 0 || oa == 1) return a.fullName < b.fullName;
            if (oa == 2 || oa == 3)
                return QVersionNumber::fromString(a.versionSuffix)
                     > QVersionNumber::fromString(b.versionSuffix);
            if (oa == 4 || oa == 5) return a.tripletPrefix < b.tripletPrefix;
            if (a.tripletPrefix != b.tripletPrefix)
                return a.tripletPrefix < b.tripletPrefix;
            return QVersionNumber::fromString(a.versionSuffix)
                 > QVersionNumber::fromString(b.versionSuffix);
        });

    bool found = false;
    for (const GnuToolchainName& c_prog : selectedCompilers) {
        Toolchain tc;
        tc.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (ToolchainDetect::detectProperties(tc, folder, c_prog, selectedIdentities[c_prog.fullName])) {
            ToolchainDetect::detectExecutables(tc, c_prog);
            ToolchainDetect::detectDirectories(tc, folder, c_prog);
            addToolchain(tc);
            found = true;
        }
    }
    for (const GnuToolchainName& c_prog : sdccCandidates) {
        Toolchain tc;
        tc.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (ToolchainDetect::detectProperties(tc, folder, c_prog)) {
            ToolchainDetect::detectExecutables(tc, c_prog);
            ToolchainDetect::detectDirectories(tc, folder, c_prog);
            addToolchain(tc);
            found = true;
        }
    }

    // Phase 2: Collect existing Clang triplet keys for deduplication
    QSet<QString> existingTripletKeys;
    for (const Toolchain& tc : mToolchains) {
        if (tc.compilerType != CompilerType::Clang
            && tc.compilerType != CompilerType::AppleClang)
            continue;
        GnuToolchainName pr = GnuToolchainName::parse(
            QFileInfo(tc.ccompiler).fileName());
        if (!pr.isValid)
            continue;
        QString variantKey;
        if (pr.hasTriplet)
            variantKey += pr.tripletPrefix + "-";
        if (pr.hasVersion)
            variantKey += pr.versionSuffix;
        existingTripletKeys.insert(variantKey);
    }

    // Phase 3: For each bare Clang (no triplet in filename),
    // scan parent directory for triplet directories
    for (const GnuToolchainName& c_prog : selectedCompilers) {
        if (!c_prog.isValid
            || c_prog.baseProgram != CLANG_PROGRAM
            || c_prog.hasTriplet)
            continue;
        QList<Toolchain> triplets = ToolchainDetect::discoverClangTriplets(
            folder, c_prog, existingTripletKeys,
            selectedIdentities[c_prog.fullName]);
        for (const Toolchain& tc : triplets) {
            addToolchain(tc);
            found = true;
        }
    }

    return found;
}
