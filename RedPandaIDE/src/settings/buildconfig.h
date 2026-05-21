#ifndef BUILDCONFIG_H
#define BUILDCONFIG_H

#include <QString>
#include <QMap>
#include <optional>
#include <memory>
#include "toolchain.h"

class BuildConfiguration {
public:
    QString     name;                           // "Release" / "Debug"
    CompilerType compilerTypeFamily = CompilerType::Unknown;

    // build-config-level compiler options
    QMap<QString,QString> compilerOptions;

    // custom params (appended to command line)
    QString     customCompileParams;
    QString     customLinkParams;

    // charset
    bool        autoAddCharsetParams = true;
    QString     execCharset;                    // ENCODING_SYSTEM_DEFAULT

    // link model override (optional: not set = inherit from Toolchain)
    std::optional<LinkModel> linkModelOverride;
};

using PBuildConfiguration = std::shared_ptr<BuildConfiguration>;

#endif // BUILDCONFIG_H
