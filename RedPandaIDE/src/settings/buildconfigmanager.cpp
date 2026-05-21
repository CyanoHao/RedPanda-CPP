#include "buildconfigmanager.h"
#include "buildconfig_json.h"

BuildConfigManager::BuildConfigManager()
{
}

QList<BuildConfiguration> BuildConfigManager::configsFor(CompilerType family) const
{
    return mConfigSets.value(family);
}

QString BuildConfigManager::activeConfigName() const
{
    return mActiveConfigName;
}

BuildConfigManager::ConfigSetMap BuildConfigManager::allConfigs() const
{
    return mConfigSets;
}

void BuildConfigManager::setActiveConfig(const QString& name)
{
    mActiveConfigName = name;
}

void BuildConfigManager::addConfig(CompilerType family, const BuildConfiguration& cfg)
{
    QList<BuildConfiguration>& list = mConfigSets[family];
    for (const auto& existing : list) {
        if (existing.name == cfg.name)
            return;
    }
    list.append(cfg);
}

void BuildConfigManager::removeConfig(CompilerType family, const QString& name)
{
    QList<BuildConfiguration>& list = mConfigSets[family];
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].name == name) {
            list.removeAt(i);
            return;
        }
    }
}

void BuildConfigManager::updateConfig(CompilerType family, const QString& name,
                                       const BuildConfiguration& cfg)
{
    QList<BuildConfiguration>& list = mConfigSets[family];
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].name == name) {
            list[i] = cfg;
            list[i].name = name; // keep original name
            return;
        }
    }
}

void BuildConfigManager::load(const QString& filename)
{
    mConfigSets = BuildConfigJson::load(filename);
}

void BuildConfigManager::save(const QString& filename) const
{
    BuildConfigJson::save(filename, mConfigSets);
}

void BuildConfigManager::createBuiltinDefaults(CompilerType family)
{
    QList<BuildConfiguration>& list = mConfigSets[family];
    if (!list.isEmpty())
        return;

    if (family == CompilerType::GCC || family == CompilerType::Clang) {
        // Release
        BuildConfiguration release;
        release.name = "Release";
        release.compilerTypeFamily = CompilerType::GCC;
        release.compilerOptions["cc_cmd_opt_optimize"] = "2";
        release.compilerOptions[CC_CMD_OPT_DEBUG_INFO] = COMPILER_OPTION_OFF;
        release.compilerOptions[CC_CMD_OPT_WARNING_ALL] = COMPILER_OPTION_ON;
        release.compilerOptions[CC_CMD_OPT_WARNING_AS_ERROR] = COMPILER_OPTION_ON;
        release.linkModelOverride = LinkModel::Dynamic;
        list.append(release);

        // Debug
        BuildConfiguration debug;
        debug.name = "Debug";
        debug.compilerTypeFamily = CompilerType::GCC;
        debug.compilerOptions["cc_cmd_opt_optimize"] = "g";
        debug.compilerOptions[CC_CMD_OPT_DEBUG_INFO] = COMPILER_OPTION_ON;
        debug.compilerOptions[CC_CMD_OPT_WARNING_ALL] = COMPILER_OPTION_ON;
        debug.compilerOptions[CC_CMD_OPT_WARNING_EXTRA] = COMPILER_OPTION_ON;
        debug.linkModelOverride = LinkModel::Dynamic;
        list.append(debug);

#ifdef Q_OS_LINUX
        // Debug with ASan
        BuildConfiguration asan;
        asan.name = "Debug with ASan";
        asan.compilerTypeFamily = CompilerType::GCC;
        asan.compilerOptions["cc_cmd_opt_optimize"] = "0";
        asan.compilerOptions[CC_CMD_OPT_DEBUG_INFO] = COMPILER_OPTION_ON;
        asan.compilerOptions[CC_CMD_OPT_ADDRESS_SANITIZER] = "address";
        asan.compilerOptions[CC_CMD_OPT_WARNING_ALL] = COMPILER_OPTION_ON;
        asan.compilerOptions[CC_CMD_OPT_WARNING_EXTRA] = COMPILER_OPTION_ON;
        list.append(asan);
#endif
    } else if (family == CompilerType::AppleClang) {
        BuildConfiguration release;
        release.name = "Release";
        release.compilerTypeFamily = CompilerType::AppleClang;
        release.compilerOptions["cc_cmd_opt_optimize"] = "2";
        release.compilerOptions[CC_CMD_OPT_DEBUG_INFO] = COMPILER_OPTION_OFF;
        release.compilerOptions[CC_CMD_OPT_WARNING_ALL] = COMPILER_OPTION_ON;
        release.compilerOptions[CC_CMD_OPT_WARNING_AS_ERROR] = COMPILER_OPTION_ON;
        release.linkModelOverride = LinkModel::Dynamic;
        list.append(release);

        BuildConfiguration debug;
        debug.name = "Debug";
        debug.compilerTypeFamily = CompilerType::AppleClang;
        debug.compilerOptions["cc_cmd_opt_optimize"] = "g";
        debug.compilerOptions[CC_CMD_OPT_DEBUG_INFO] = COMPILER_OPTION_ON;
        debug.compilerOptions[CC_CMD_OPT_WARNING_ALL] = COMPILER_OPTION_ON;
        debug.compilerOptions[CC_CMD_OPT_WARNING_EXTRA] = COMPILER_OPTION_ON;
        debug.linkModelOverride = LinkModel::Dynamic;
        list.append(debug);
    }
#ifdef ENABLE_SDCC
    else if (family == CompilerType::SDCC) {
        BuildConfiguration release;
        release.name = "Release";
        release.compilerTypeFamily = CompilerType::SDCC;
        release.compilerOptions[SDCC_CMD_OPT_PROCESSOR] = "mcs51";
        release.compilerOptions[SDCC_OPT_MEMORY_MODEL] = "large";
        list.append(release);

        BuildConfiguration debug;
        debug.name = "Debug";
        debug.compilerTypeFamily = CompilerType::SDCC;
        debug.compilerOptions[SDCC_CMD_OPT_PROCESSOR] = "mcs51";
        debug.compilerOptions[SDCC_OPT_MEMORY_MODEL] = "large";
        list.append(debug);
    }
#endif
}
