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
#ifndef DEVCPPPROJECT_H
#define DEVCPPPROJECT_H

#include "project.h"
#include "projectoptions.h"

enum class DevCppProjectType {
    GUI=0,
    Console=1,
    StaticLib=2,
    DynamicLib=3,
    MicroController=4,
};

enum class DevCppProjectBuildBackend {
    DevCppMakefile = 0,
    FullyExpandedMakefile = 1,
    BuiltInScheduler = 2,
};

struct DevCppProjectOptions: public ProjectOptionsBase {
    explicit DevCppProjectOptions();
    DevCppProjectType type;
    DevCppProjectBuildBackend buildBackend;
    int version;
    QString compilerCmd;
    QString cppCompilerCmd;
    QString linkerCmd;
    QString resourceCmd;
    QStringList binDirs;
    QStringList includeDirs;
    QStringList libDirs;
    QString privateResource;
    QStringList resourceIncludes;
    QStringList makeIncludes;
    bool isCpp;
    QString icon;
    QString exeOutput;
    QString objectOutput;
    QString logOutput;
    bool logOutputEnabled;
    bool useCustomMakefile;
    QString customMakefile;
    bool usePrecompiledHeader;
    QString precompiledHeader;
    bool overrideOutput;
    QString overridenOutput;
    QString hostApplication;
    bool includeVersionInfo;
    bool supportXPThemes;
    int compilerSet;
    QMap<QString,QString> compilerOptions;
    ProjectVersionInfo versionInfo;
    QString cmdLineArgs;
    bool staticLink;
    bool addCharset;
    QByteArray execEncoding;
    QByteArray encoding;
    ProjectModelType modelType;
    ProjectClassBrowserType classBrowserType;
    bool allowParallelBuilding;
    int parellelBuildingJobs;
};

class DevCppProject : public BuiltInProjectBase
{
    Q_OBJECT
public:
    explicit DevCppProject(const QString& filename, const QString& name,
                     EditorList* editorList,
                     QFileSystemWatcher* fileSystemWatcher,
                     QObject *parent = nullptr);

    static std::shared_ptr<DevCppProject> load(const QString& filename,
                                    EditorList* editorList,
                                    QFileSystemWatcher* fileSystemWatcher,
                                    QObject *parent = nullptr);
    static std::shared_ptr<DevCppProject> create(const QString& filename,
                                           const QString& name,
                                           EditorList* editorList,
                                           QFileSystemWatcher* fileSystemWatcher,
                                           const std::shared_ptr<ProjectTemplate> pTemplate,
                                           bool useCpp,
                                           QObject *parent = nullptr);

    ~DevCppProject();

    QString executable() const override;
    QString makeFileName();
    QString xmakeFileName();
    bool unitsModifiedSince(const QDateTime& time);
    bool modified() const;
    bool modifiedSince(const QDateTime& time);
//    void setFileName(QString value);
    void setModified(bool value);

    PProjectModelNode addFolder(PProjectModelNode parentFolder, const QString& s);
    PProjectUnit  newUnit(PProjectModelNode parentNode,
                 const QString& customFileName="");
    PProjectUnit addUnit(const QString& inFileName,
                PProjectModelNode parentNode);
    QString folder();
    void buildPrivateResource();
    void closeUnit(PProjectUnit& unit);
    PProjectUnit doAutoOpen();
    bool fileAlreadyExists(const QString& s);

    QString getNodePath(PProjectModelNode node);
    void incrementBuildNumber();

    Editor* openUnit(PProjectUnit& unit, bool forceOpen=true);
    Editor* openUnit(PProjectUnit& unit, const PProjectEditorLayout& layout);
    Editor* unitEditor(const PProjectUnit& unit) const;
    Editor* unitEditor(const ProjectUnit* unit) const;

    QList<PProjectUnit> unitList();
    QStringList unitFiles();

    PProjectModelNode pointerToNode(ProjectModelNode * p, PProjectModelNode parent=PProjectModelNode());
    void rebuildNodes();
    bool removeUnit(PProjectUnit& unit, bool doClose, bool removeFile = false);
    bool removeFolder(PProjectModelNode node);
    void resetParserProjectFiles();
    void saveAll(); // save [Project] and  all [UnitX]
    void saveLayout(); // save all [UnitX]
    void saveOptions();
    void renameUnit(PProjectUnit& unit, const QString& sFileName);
    bool saveUnits();

    PProjectUnit findUnit(const QString& filename);
    PProjectUnit findUnit(const Editor* editor);

    void associateEditor(Editor* editor);
    void associateEditorToUnit(Editor* editor, PProjectUnit unit);
    bool setCompileOption(const QString &key, const QString &value);
    QString getCompileOption(const QString &key) const;

    void updateFolders();
    void setCompilerSet(int compilerSetIndex);

    bool saveAsTemplate(const QString& templateFolder,
                        const QString& name,
                        const QString& description,
                        const QString& category);

    void setEncoding(const QByteArray& encoding);

    std::shared_ptr<CppParser> cppParser();
    const QString &filename() const;

    const QString &name() const;
    void setName(const QString &newName);

    const PProjectModelNode &rootNode() const;

    DevCppProjectOptions &options();

    ProjectModel* model() ;

    ProjectModelType modelType() const;
    void setModelType(ProjectModelType type);

    EditorList *editorList() const;

    QFileSystemWatcher *fileSystemWatcher() const;

    QString fileSystemNodeFolderPath(const PProjectModelNode& node);

    QStringList binDirs();

    void renameFolderNode(PProjectModelNode node, const QString newName);
    void loadUnitLayout(Editor *e);
signals:
    void unitRemoved(const QString& fileName);
    void unitAdded(const QString& fileName);
    void unitRenamed(const QString& oldFileName, const QString& newFileName);
    void nodeRenamed();
    void modifyChanged(bool value);

private:
    bool internalRemoveUnit(PProjectUnit& unit, bool doClose, bool removeFile);
    PProjectUnit internalAddUnit(const QString& inFileName,
                PProjectModelNode parentNode);

    bool assignTemplate(const std::shared_ptr<ProjectTemplate> aTemplate, bool useCpp);
    void checkProjectFileForUpdate(SimpleIni& ini);
    void createFolderNodes();
    void createFileSystemFolderNodes();
    void createFileSystemFolderNode(ProjectModelNodeType folderType, const QString& folderName, PProjectModelNode parent, const QSet<QString>& validFolders);
    PProjectModelNode getParentFileSystemFolderNode(const QString& filename);
    PProjectModelNode findFileSystemFolderNode(const QString& folderPath, ProjectModelNodeType nodeType);
    PProjectModelNode getCustomeFolderNodeFromName(const QString& name);
    void loadOptions(SimpleIni& ini);
    //PProjectUnit
    QHash<QString, PProjectEditorLayout> loadLayout();

    PProjectModelNode makeNewFolderNode(
            const QString& folderName,
            PProjectModelNode newParent,
            ProjectModelNodeType nodeType=ProjectModelNodeType::Folder,
            int priority=0);
    PProjectModelNode makeNewFileNode(
            //const QString& fileName,
            PProjectUnit unit,
            int priority,
            PProjectModelNode newParent);
    PProjectModelNode makeProjectNode();
    void open();
    void removeFolderRecurse(PProjectModelNode node);
    void updateFolderNode(PProjectModelNode node);
    void updateCompilerSetting();

private:
    DevCppProjectOptions mOptions;
    bool mModified;

    QHash<ProjectModelNodeType, PProjectModelNode> mSpecialNodes;
    QHash<QString, PProjectModelNode> mFileSystemFolderNodes;

    QList<PProjectModelNode> mCustomFolderNodes;
    ProjectModel mModel;
    EditorList *mEditorList;
    QFileSystemWatcher* mFileSystemWatcher;
};

#endif // DEVCPPPROJECT_H
