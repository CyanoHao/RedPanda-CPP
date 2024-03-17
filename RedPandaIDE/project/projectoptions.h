/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PROJECTOPTIONS_H
#define PROJECTOPTIONS_H

#include <QMap>
#include <QWidget>
#include "compiler/compilerinfo.h"

enum class ProjectModelType {
    FileSystem,
    Custom
};

enum class ProjectClassBrowserType {
    CurrentFile,
    WholeProject
};

struct ProjectVersionInfo{
    explicit ProjectVersionInfo();
    int major;
    int minor;
    int release;
    int build;
    int languageID;
    int charsetID;
    QString companyName;
    QString fileVersion;
    QString fileDescription;
    QString internalName;
    QString legalCopyright;
    QString legalTrademarks;
    QString originalFilename;
    QString productName;
    QString productVersion;
    bool autoIncBuildNr;
    bool syncProduct;
};

struct ProjectOptionsBase {
};

#endif // PROJECTOPTIONS_H
