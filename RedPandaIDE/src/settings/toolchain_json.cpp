#include "toolchain_json.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QUuid>
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

static LinkModel linkModelFromString(const QString& s)
{
    if (s == "static")      return LinkModel::Static;
    if (s == "pie")         return LinkModel::PIE;
    if (s == "static-pie")  return LinkModel::StaticPIE;
    return LinkModel::Dynamic;
}

static QString linkModelToString(LinkModel m)
{
    switch (m) {
    case LinkModel::Static:     return "static";
    case LinkModel::PIE:        return "pie";
    case LinkModel::StaticPIE:  return "static-pie";
    default:                    return "dynamic";
    }
}

static QStringList jsonArrayToStringList(const QJsonArray& arr)
{
    QStringList result;
    for (const auto& v : arr)
        result << v.toString();
    return result;
}

static QJsonArray stringListToJsonArray(const QStringList& list)
{
    QJsonArray arr;
    for (const auto& s : list)
        arr << s;
    return arr;
}

static QJsonObject optionsToJson(const QMap<QString,QString>& options)
{
    QJsonObject obj;
    for (auto it = options.begin(); it != options.end(); ++it)
        obj[it.key()] = it.value();
    return obj;
}

static QMap<QString,QString> optionsFromJson(const QJsonObject& obj)
{
    QMap<QString,QString> result;
    for (auto it = obj.begin(); it != obj.end(); ++it)
        result[it.key()] = it.value().toString();
    return result;
}

namespace ToolchainJson {

Toolchain fromJson(const QJsonObject& obj)
{
    Toolchain tc;
    tc.id = obj["id"].toString();
    if (tc.id.isEmpty())
        tc.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    tc.name = obj["name"].toString();
    tc.compilerType = compilerTypeFromString(obj["compilerType"].toString());
    tc.ccompiler = obj["ccompiler"].toString();
    tc.cppCompiler = obj["cppcompiler"].toString();
    tc.make = obj["make"].toString();
    tc.debugger = obj["debugger"].toString();
    tc.resourceCompiler = obj["resourceCompiler"].toString();
    tc.debugServer = obj["debugServer"].toString();
    tc.binDirs = jsonArrayToStringList(obj["binDirs"].toArray());
    tc.cIncludeDirs = jsonArrayToStringList(obj["cIncludeDirs"].toArray());
    tc.cppIncludeDirs = jsonArrayToStringList(obj["cppIncludeDirs"].toArray());
    tc.libDirs = jsonArrayToStringList(obj["libDirs"].toArray());
    tc.dumpMachine = obj["dumpMachine"].toString();
    tc.version = obj["version"].toString();
    tc.target = obj["target"].toString();
    tc.clangTarget = obj["clangTarget"].toString();
    tc.forceEnglishOutput = obj["forceEnglishOutput"].toBool();
    tc.preprocessingSuffix = obj["preprocessingSuffix"].toString(".i");
    tc.compilationProperSuffix = obj["compilationProperSuffix"].toString(".s");
    tc.assemblingSuffix = obj["assemblingSuffix"].toString(".o");
    tc.executableSuffix = obj["executableSuffix"].toString();
    tc.defaultLinkModel = linkModelFromString(obj["defaultLinkModel"].toString());
    tc.compilerOptions = optionsFromJson(obj["compilerOptions"].toObject());
    return tc;
}

QJsonObject toJson(const Toolchain& tc)
{
    QJsonObject obj;
    obj["id"] = tc.id;
    obj["name"] = tc.name;
    obj["compilerType"] = compilerTypeToString(tc.compilerType);
    obj["ccompiler"] = tc.ccompiler;
    obj["cppcompiler"] = tc.cppCompiler;
    obj["make"] = tc.make;
    obj["debugger"] = tc.debugger;
    obj["resourceCompiler"] = tc.resourceCompiler;
    obj["debugServer"] = tc.debugServer;
    obj["binDirs"] = stringListToJsonArray(tc.binDirs);
    obj["cIncludeDirs"] = stringListToJsonArray(tc.cIncludeDirs);
    obj["cppIncludeDirs"] = stringListToJsonArray(tc.cppIncludeDirs);
    obj["libDirs"] = stringListToJsonArray(tc.libDirs);
    obj["dumpMachine"] = tc.dumpMachine;
    obj["version"] = tc.version;
    obj["target"] = tc.target;
    obj["clangTarget"] = tc.clangTarget;
    obj["forceEnglishOutput"] = tc.forceEnglishOutput;
    obj["preprocessingSuffix"] = tc.preprocessingSuffix;
    obj["compilationProperSuffix"] = tc.compilationProperSuffix;
    obj["assemblingSuffix"] = tc.assemblingSuffix;
    obj["executableSuffix"] = tc.executableSuffix;
    obj["defaultLinkModel"] = linkModelToString(tc.defaultLinkModel);
    obj["compilerOptions"] = optionsToJson(tc.compilerOptions);
    return obj;
}

QList<Toolchain> load(const QString& filename)
{
    QList<Toolchain> result;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[ToolchainJson] cannot open" << filename;
        return result;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject root = doc.object();
    QJsonArray arr = root["toolchains"].toArray();
    for (const auto& v : arr) {
        result.append(fromJson(v.toObject()));
    }
    return result;
}

bool save(const QString& filename, const QList<Toolchain>& toolchains)
{
    QJsonObject root;
    root["formatVersion"] = 1;
    QJsonArray arr;
    for (const auto& tc : toolchains) {
        arr.append(toJson(tc));
    }
    root["toolchains"] = arr;
    root["defaultId"] = toolchains.isEmpty() ? QString() : toolchains.first().id;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[ToolchainJson] cannot write" << filename;
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace ToolchainJson
