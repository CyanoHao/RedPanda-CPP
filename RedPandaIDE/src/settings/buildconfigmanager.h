#ifndef BUILDCONFIGMANAGER_H
#define BUILDCONFIGMANAGER_H

#include "buildconfig.h"
#include <QMap>
#include <QList>
#include <QString>

class BuildConfigManager {
public:
    BuildConfigManager();

    using ConfigSetMap = QMap<CompilerType, QList<BuildConfiguration>>;

    QList<BuildConfiguration> configsFor(CompilerType family) const;
    QString activeConfigName() const;
    ConfigSetMap allConfigs() const;

    void setActiveConfig(const QString& name);
    void addConfig(CompilerType family, const BuildConfiguration& cfg);
    void removeConfig(CompilerType family, const QString& name);
    void updateConfig(CompilerType family, const QString& name,
                      const BuildConfiguration& cfg);

    void load(const QString& filename);
    void save(const QString& filename) const;

    void createBuiltinDefaults(CompilerType family);

private:
    ConfigSetMap mConfigSets;
    QString mActiveConfigName;
};

#endif // BUILDCONFIGMANAGER_H
