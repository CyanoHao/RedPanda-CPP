#ifndef BUILDCONFIG_JSON_H
#define BUILDCONFIG_JSON_H

#include "buildconfig.h"
#include <QMap>
#include <QList>
#include <QString>
#include <QJsonObject>

namespace BuildConfigJson {

using ConfigSetMap = QMap<CompilerType, QList<BuildConfiguration>>;

ConfigSetMap load(const QString& filename);
bool save(const QString& filename, const ConfigSetMap& configSets);

BuildConfiguration fromJson(const QJsonObject& obj);
QJsonObject toJson(const BuildConfiguration& cfg);

} // namespace BuildConfigJson

#endif // BUILDCONFIG_JSON_H
