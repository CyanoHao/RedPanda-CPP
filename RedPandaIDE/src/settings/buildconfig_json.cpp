#include "buildconfig_json.h"
#include "toolchain_json.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

static CompilerType compilerTypeFromString(const QString& s)
{
    if (s == "GCC")        return CompilerType::GCC;
    if (s == "Clang")      return CompilerType::Clang;
    if (s == "AppleClang") return CompilerType::AppleClang;
#ifdef ENABLE_SDCC
    if (s == "SDCC")       return CompilerType::SDCC;
#endif
    return CompilerType::Unknown;
}

static QString compilerTypeToString(CompilerType type)
{
    switch (type) {
    case CompilerType::GCC:        return "GCC";
    case CompilerType::Clang:      return "Clang";
    case CompilerType::AppleClang: return "AppleClang";
#ifdef ENABLE_SDCC
    case CompilerType::SDCC:       return "SDCC";
#endif
    default:                       return "Unknown";
    }
}

static QJsonObject stringMapToJson(const QMap<QString,QString>& map)
{
    QJsonObject obj;
    for (auto it = map.begin(); it != map.end(); ++it)
        obj[it.key()] = it.value();
    return obj;
}

static QMap<QString,QString> stringMapFromJson(const QJsonObject& obj)
{
    QMap<QString,QString> result;
    for (auto it = obj.begin(); it != obj.end(); ++it)
        result[it.key()] = it.value().toString();
    return result;
}

namespace BuildConfigJson {

BuildConfiguration fromJson(const QJsonObject& obj)
{
    BuildConfiguration cfg;
    cfg.name = obj["name"].toString();
    cfg.compilerTypeFamily = compilerTypeFromString(
        obj["compilerType"].toString("GCC"));
    cfg.compilerOptions = stringMapFromJson(obj["compilerOptions"].toObject());
    cfg.customCompileParams = obj["customCompileParams"].toString();
    cfg.customLinkParams = obj["customLinkParams"].toString();
    cfg.autoAddCharsetParams = obj["autoAddCharsetParams"].toBool(true);
    cfg.execCharset = obj["execCharset"].toString();

    if (obj.contains("linkModel")) {
        QString lm = obj["linkModel"].toString();
        if (lm == "static")      cfg.linkModelOverride = LinkModel::Static;
        else if (lm == "pie")    cfg.linkModelOverride = LinkModel::PIE;
        else if (lm == "static-pie") cfg.linkModelOverride = LinkModel::StaticPIE;
        else if (lm == "dynamic")    cfg.linkModelOverride = LinkModel::Dynamic;
    }
    return cfg;
}

QJsonObject toJson(const BuildConfiguration& cfg)
{
    QJsonObject obj;
    obj["name"] = cfg.name;
    obj["compilerType"] = compilerTypeToString(cfg.compilerTypeFamily);
    obj["compilerOptions"] = stringMapToJson(cfg.compilerOptions);
    obj["customCompileParams"] = cfg.customCompileParams;
    obj["customLinkParams"] = cfg.customLinkParams;
    obj["autoAddCharsetParams"] = cfg.autoAddCharsetParams;
    obj["execCharset"] = cfg.execCharset;

    if (cfg.linkModelOverride.has_value()) {
        switch (cfg.linkModelOverride.value()) {
        case LinkModel::Static:     obj["linkModel"] = "static"; break;
        case LinkModel::PIE:        obj["linkModel"] = "pie"; break;
        case LinkModel::StaticPIE:  obj["linkModel"] = "static-pie"; break;
        default:                    obj["linkModel"] = "dynamic"; break;
        }
    }
    return obj;
}

ConfigSetMap load(const QString& filename)
{
    ConfigSetMap result;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[BuildConfigJson] cannot open" << filename;
        return result;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonObject sets = root["configSets"].toObject();
    for (auto it = sets.begin(); it != sets.end(); ++it) {
        CompilerType family = compilerTypeFromString(it.key());
        QJsonObject familyObj = it.value().toObject();
        QJsonArray configs = familyObj["configs"].toArray();
        QList<BuildConfiguration> list;
        for (const auto& v : configs) {
            list.append(fromJson(v.toObject()));
        }
        result[family] = list;
    }
    return result;
}

bool save(const QString& filename, const ConfigSetMap& configSets)
{
    QJsonObject root;
    root["formatVersion"] = 1;

    QJsonObject sets;
    for (auto it = configSets.begin(); it != configSets.end(); ++it) {
        QJsonObject familyObj;
        QJsonArray configs;
        for (const auto& cfg : it.value()) {
            configs.append(toJson(cfg));
        }
        familyObj["configs"] = configs;
        familyObj["activeConfig"] = it.value().isEmpty()
            ? QString() : it.value().first().name;
        sets[compilerTypeToString(it.key())] = familyObj;
    }
    root["configSets"] = sets;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[BuildConfigJson] cannot write" << filename;
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace BuildConfigJson
