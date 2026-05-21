#ifndef TOOLCHAIN_JSON_H
#define TOOLCHAIN_JSON_H

#include "toolchain.h"
#include <QList>
#include <QString>
#include <QJsonObject>

namespace ToolchainJson {

QList<Toolchain> load(const QString& filename);
bool save(const QString& filename, const QList<Toolchain>& toolchains);

Toolchain fromJson(const QJsonObject& obj);
QJsonObject toJson(const Toolchain& tc);

} // namespace ToolchainJson

#endif // TOOLCHAIN_JSON_H
