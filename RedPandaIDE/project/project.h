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
#ifndef PROJECT_H
#define PROJECT_H

#include <QAbstractItemModel>
#include <QHash>
#include <QSet>
#include <QSortFilterProxyModel>
#include <memory>
#include <QObject>
#include <QString>

#include "projectoptions.h"

class DevCppProject;
class Editor;
class CppParser;
class EditorList;
class QFileSystemWatcher;

enum ProjectModelNodeType {
    DUMMY_HEADERS_FOLDER,
    DUMMY_SOURCES_FOLDER,
    DUMMY_OTHERS_FOLDER,
    Folder,
    File
};

struct ProjectModelItemRecord {
    ProjectModelNodeType type;
    QString fullPath;
};

class ProjectUnit;

struct ProjectModelNode;
using PProjectModelNode = std::shared_ptr<ProjectModelNode>;
struct ProjectModelNode {
    QString text;
    std::weak_ptr<ProjectModelNode> parent;
    bool isUnit;
    std::weak_ptr<ProjectUnit> pUnit;
    int priority;
    QList<PProjectModelNode>  children;
    ProjectModelNodeType folderNodeType;
    int level;
};

struct ProjectEditorLayout {
    QString filename;
    int topLine;
    int left;
    int caretX;
    int caretY;
    int order;
    bool isFocused;
    bool isOpen;
};

using PProjectEditorLayout = std::shared_ptr<ProjectEditorLayout>;

class ProjectUnit {

public:
    explicit ProjectUnit(DevCppProject* parent);
    DevCppProject* parent() const;
    const QString &fileName() const;
    void setFileName(QString newFileName);
    const QString &folder() const;
    void setFolder(const QString &newFolder);
    bool compile() const;
    void setCompile(bool newCompile);
    bool compileCpp() const;
    void setCompileCpp(bool newCompileCpp);
    bool overrideBuildCmd() const;
    void setOverrideBuildCmd(bool newOverrideBuildCmd);
    const QString &buildCmd() const;
    void setBuildCmd(const QString &newBuildCmd);
    bool link() const;
    void setLink(bool newLink);
    int priority() const;
    void setPriority(int newPriority);
    const QByteArray &encoding() const;
    void setEncoding(const QByteArray &newEncoding);

    PProjectModelNode &node();
    void setNode(const PProjectModelNode &newNode);

//    bool FileMissing() const;
//    void setFileMissing(bool newDontSave);

    void setNew(bool newNew);

    const QByteArray &realEncoding() const;
    void setRealEncoding(const QByteArray &newRealEncoding);

private:
    DevCppProject* mParent;
    QString mFileName;
    QString mFolder;
    bool mNew;
    bool mCompile;
    bool mCompileCpp;
    bool mOverrideBuildCmd;
    QString mBuildCmd;
    bool mLink;
    int mPriority;
    QByteArray mEncoding;
    QByteArray mRealEncoding;
    PProjectModelNode mNode;
//    bool mFileMissing;
};

using PProjectUnit = std::shared_ptr<ProjectUnit>;

class GitRepository;
class CustomFileIconProvider;
class ProjectModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit ProjectModel(DevCppProject* project, QObject* parent=nullptr);
    ~ProjectModel();
    void beginUpdate();
    void endUpdate();
private:
    DevCppProject* mProject;
    int mUpdateCount;
    CustomFileIconProvider* mIconProvider;


    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void refreshIcon(const QModelIndex& index, bool update=true);
    void refreshIcon(const QString& filename);
    void refreshIcons();
    void refreshNodeIconRecursive(PProjectModelNode node);

    QModelIndex getNodeIndex(ProjectModelNode *node) const;
    QModelIndex getParentIndex(ProjectModelNode * node) const;

    QModelIndex rootIndex() const;

private:

    // QAbstractItemModel interface
public:
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    DevCppProject *project() const;
    CustomFileIconProvider *iconProvider() const;

    // QAbstractItemModel interface
public:
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
};

class ProjectModelSortFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ProjectModelSortFilterProxy(QObject *parent = nullptr);
    // QSortFilterProxyModel interface
protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

class ProjectTemplate;

class ProjectBase: public QObject {
    Q_OBJECT
public:
    ProjectBase(const QString &filename, const QString &name,
                 EditorList* editorList,
                 QFileSystemWatcher* fileSystemWatcher,
                 QObject *parent = nullptr);

    virtual bool writeable() const = 0;

    QString directory() const;
    virtual QString executable() const = 0;

protected:
    QString relativePath(const QString &filename);
    QStringList relativePaths(const QStringList &files);
    QString absolutePath(const QString &filename);
    QStringList absolutePaths(const QStringList &files);

protected:
    ProjectOptionsBase mOptions;
    QHash<QString,PProjectUnit> mUnits;
    QString mFilename;
    QString mName;
    QString mDirectory;
    QStringList mFolders;
    std::shared_ptr<CppParser> mParser;
    PProjectModelNode mRootNode;
};

class BuiltInProjectBase : public ProjectBase {
    Q_OBJECT
public:
    explicit BuiltInProjectBase(const QString &filename, const QString &name,
                            EditorList* editorList,
                            QFileSystemWatcher* fileSystemWatcher,
                            QObject *parent = nullptr);

public:

protected:

};

#endif // PROJECT_H
