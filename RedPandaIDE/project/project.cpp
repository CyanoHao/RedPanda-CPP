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
#include "project.h"
#include "editor.h"
#include "utils.h"
#include "systemconsts.h"
#include "editorlist.h"
#include <parser/cppparser.h>
#include "utils.h"
#include "qt_utils/charsetinfo.h"
#include "projecttemplate.h"
#include "systemconsts.h"
#include "iconsmanager.h"

#include <QFileSystemWatcher>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>
#include <QMessageBox>
#include <QDirIterator>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "customfileiconprovider.h"
#include <QMimeData>
#include "settings.h"
#include "vcs/gitrepository.h"

QString ProjectBase::directory() const
{
    return mDirectory;
}

QString ProjectBase::relativePath(const QString &filename)
{
    QString appPath = includeTrailingPathDelimiter(pSettings->dirs().appDir());
    QString projectPath = includeTrailingPathDelimiter(directory());
    if (filename.startsWith(appPath) && !filename.startsWith(projectPath)) {
        return "%APP_PATH%/"+filename.mid(appPath.length());
    }
    QDir projectDir(directory());
    QDir grandparentDir(projectDir.absoluteFilePath("../../"));
    QString grandparentPath=grandparentDir.absolutePath();
    if (grandparentDir.exists()
            && filename.startsWith(grandparentPath))
        return extractRelativePath(directory(),filename);
    return filename;
}

QStringList ProjectBase::relativePaths(const QStringList &files)
{
    QStringList lst;
    for (const QString &file : files) {
        lst.append(relativePath(file));
    }
    return lst;
}

QString ProjectBase::absolutePath(const QString &filename)
{
    QString appSuffix = "%APP_PATH%/";
    if (filename.startsWith(appSuffix)) {
        return includeTrailingPathDelimiter(pSettings->dirs().appDir()) + filename.mid(appSuffix.length());
    }
    return generateAbsolutePath(directory(),filename);
}

QStringList ProjectBase::absolutePaths(const QStringList &files)
{
    QStringList lst;
    for (const QString &file : files) {
        lst.append(absolutePath(file));
    }
    return lst;
}

ProjectUnit::ProjectUnit(DevCppProject* parent)
{
    mNode = nullptr;
    mParent = parent;
//    mFileMissing = false;
    mPriority=0;
    mNew = true;
    mEncoding=ENCODING_PROJECT;
    mRealEncoding="";
}

DevCppProject *ProjectUnit::parent() const
{
    return mParent;
}

const QString &ProjectUnit::fileName() const
{
    return mFileName;
}

void ProjectUnit::setFileName(QString newFileName)
{
    newFileName = QFileInfo(newFileName).absoluteFilePath();
    if (mFileName != newFileName) {
        mFileName = newFileName;
        if (mNode) {
            mNode->text = extractFileName(mFileName);
        }
    }
}

void ProjectUnit::setNew(bool newNew)
{
    mNew = newNew;
}

const QByteArray &ProjectUnit::realEncoding() const
{
    return mRealEncoding;
}

void ProjectUnit::setRealEncoding(const QByteArray &newRealEncoding)
{
    mRealEncoding = newRealEncoding;
}

const QString &ProjectUnit::folder() const
{
    return mFolder;
}

void ProjectUnit::setFolder(const QString &newFolder)
{
    mFolder = newFolder;
}

bool ProjectUnit::compile() const
{
    return mCompile;
}

void ProjectUnit::setCompile(bool newCompile)
{
    mCompile = newCompile;
}

bool ProjectUnit::compileCpp() const
{
    return mCompileCpp;
}

void ProjectUnit::setCompileCpp(bool newCompileCpp)
{
    mCompileCpp = newCompileCpp;
}

bool ProjectUnit::overrideBuildCmd() const
{
    return mOverrideBuildCmd;
}

void ProjectUnit::setOverrideBuildCmd(bool newOverrideBuildCmd)
{
    mOverrideBuildCmd = newOverrideBuildCmd;
}

const QString &ProjectUnit::buildCmd() const
{
    return mBuildCmd;
}

void ProjectUnit::setBuildCmd(const QString &newBuildCmd)
{
    mBuildCmd = newBuildCmd;
}

bool ProjectUnit::link() const
{
    return mLink;
}

void ProjectUnit::setLink(bool newLink)
{
    mLink = newLink;
}

int ProjectUnit::priority() const
{
    return mPriority;
}

void ProjectUnit::setPriority(int newPriority)
{
    if (mPriority!=newPriority) {
        mPriority = newPriority;
        if (mNode)
            mNode->priority = mPriority;
    }
}

const QByteArray &ProjectUnit::encoding() const
{
    return mEncoding;
}

void ProjectUnit::setEncoding(const QByteArray &newEncoding)
{
    if (mEncoding != newEncoding) {
        Editor * editor=mParent->unitEditor(this);
        if (editor) {
            editor->setEncodingOption(newEncoding);
        }
        mEncoding = newEncoding;
    }
}

PProjectModelNode &ProjectUnit::node()
{
    return mNode;
}

void ProjectUnit::setNode(const PProjectModelNode &newNode)
{
    mNode = newNode;
}

//bool ProjectUnit::FileMissing() const
//{
//    return mFileMissing;
//}

//void ProjectUnit::setFileMissing(bool newDontSave)
//{
//    mFileMissing = newDontSave;
//}

ProjectModel::ProjectModel(DevCppProject *project, QObject *parent):
    QAbstractItemModel(parent),
    mProject(project)
{
    mUpdateCount = 0;
    //delete in the destructor
    mIconProvider = new CustomFileIconProvider();
}

ProjectModel::~ProjectModel()
{
    delete mIconProvider;
}

void ProjectModel::beginUpdate()
{
    if (mUpdateCount==0) {
        beginResetModel();
    }
    mUpdateCount++;
}

void ProjectModel::endUpdate()
{
    mUpdateCount--;
    if (mUpdateCount==0) {
        mIconProvider->setRootFolder(mProject->folder());
        endResetModel();
    }
}

CustomFileIconProvider *ProjectModel::iconProvider() const
{
    return mIconProvider;
}

bool ProjectModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent,row,row+count-1);
    endInsertRows();
    return true;
}

bool ProjectModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent,row,row+count-1);
    if (!parent.isValid())
        return false;
    ProjectModelNode* parentNode = static_cast<ProjectModelNode*>(parent.internalPointer());
    if (!parentNode)
        return false;

    parentNode->children.removeAt(row);

    endRemoveRows();
    return true;
}

DevCppProject *ProjectModel::project() const
{
    return mProject;
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row,column,mProject->rootNode().get());
    }
    ProjectModelNode* parentNode = static_cast<ProjectModelNode*>(parent.internalPointer());
    if (!parentNode) {
        return QModelIndex();
    }
    if (row<0 || row>=parentNode->children.count())
        return QModelIndex();
    return createIndex(row,column,parentNode->children[row].get());
}

QModelIndex ProjectModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    ProjectModelNode * node = static_cast<ProjectModelNode*>(child.internalPointer());
    if (!node)
        return QModelIndex();
    return getParentIndex(node);
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;
    ProjectModelNode* p = static_cast<ProjectModelNode*>(parent.internalPointer());
    if (p) {
        return p->children.count();
    } else {
        return mProject->rootNode()->children.count();
    }
}

int ProjectModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    ProjectModelNode* p = static_cast<ProjectModelNode*>(index.internalPointer());
    if (!p)
        return QVariant();
    if (role == Qt::DisplayRole) {
#ifdef ENABLE_VCS
        if (p == mProject->rootNode().get()) {
            QString branch;
            if (mIconProvider->VCSRepository()->hasRepository(branch))
                return QString("%1 [%2]").arg(p->text,branch);
        }
#endif
        return p->text;
    } else if (role==Qt::EditRole) {
        return p->text;
    } else if (role == Qt::DecorationRole) {
        QIcon icon;
        if (p->isUnit) {
            PProjectUnit unit = p->pUnit.lock();
            if (unit)
                icon = mIconProvider->icon(unit->fileName());
        } else {
            if (p == mProject->rootNode().get()) {
#ifdef ENABLE_VCS
                QString branch;
                if (mIconProvider->VCSRepository()->hasRepository(branch))
                    icon = pIconsManager->getIcon(IconsManager::FILESYSTEM_GIT);
#endif
            } else {
                switch(p->folderNodeType) {
                case ProjectModelNodeType::DUMMY_HEADERS_FOLDER:
                    icon = pIconsManager->getIcon(IconsManager::FILESYSTEM_HEADERS_FOLDER);
                    break;
                case ProjectModelNodeType::DUMMY_SOURCES_FOLDER:
                    icon = pIconsManager->getIcon(IconsManager::FILESYSTEM_SOURCES_FOLDER);
                    break;
                default:
                    icon = pIconsManager->getIcon(IconsManager::FILESYSTEM_FOLDER);
                }
            }
            if (icon.isNull())
                icon = mIconProvider->icon(QFileIconProvider::Folder);
        }
        return icon;
    }
    return QVariant();
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    ProjectModelNode* p = static_cast<ProjectModelNode*>(index.internalPointer());
    if (!p)
        return Qt::NoItemFlags;
    if (p==mProject->rootNode().get())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
    if (mProject && mProject->modelType() == ProjectModelType::FileSystem) {
        Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (p->isUnit)
            flags.setFlag(Qt::ItemIsEditable);
        return flags;
    } else {
        Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
        if (!p->isUnit) {
            flags.setFlag(Qt::ItemIsDropEnabled);
            flags.setFlag(Qt::ItemIsDragEnabled,false);
        }
        return flags;
    }
}

bool ProjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    ProjectModelNode* p = static_cast<ProjectModelNode*>(index.internalPointer());
    PProjectModelNode node = mProject->pointerToNode(p);
    if (!node)
        return false;
    if (role == Qt::EditRole) {
        if (node == mProject->rootNode()) {
            QString newName = value.toString().trimmed();
            if (newName.isEmpty())
                return false;
            mProject->setName(newName);
            emit dataChanged(index,index);
            return true;
        }
        PProjectUnit unit = node->pUnit.lock();
        if (unit) {
            //change unit name

            QString newName = value.toString().trimmed();
            if (newName.isEmpty())
                return false;
            if (newName ==  node->text)
                return false;
            QString oldName = unit->fileName();
            QString curDir = extractFilePath(oldName);
            newName = generateAbsolutePath(curDir,newName);
            // Only continue if the user says so...
            if (fileExists(newName) && newName.compare(oldName, PATH_SENSITIVITY)!=0) {
                // don't remove when changing case for example
                if (QMessageBox::question(nullptr,
                                          tr("File exists"),
                                          tr("File '%1' already exists. Delete it now?")
                                          .arg(newName),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No) == QMessageBox::Yes) {
                    // Close the target file...
                    Editor * e=mProject->editorList()->getOpenedEditorByFilename(newName);
                    if (e)
                        mProject->editorList()->closeEditor(e);

                    // Remove it from the current project...
                    PProjectUnit unit = mProject->findUnit(newName);
                    if (unit) {
                        mProject->removeUnit(unit,false);
                    }

                    // All references to the file are removed. Delete the file from disk
                    if (!QFile::remove(newName)) {
                        QMessageBox::critical(nullptr,
                                              tr("Remove failed"),
                                              tr("Failed to remove file '%1'")
                                              .arg(newName),
                                              QMessageBox::Ok);
                        return false;
                    }
                } else {
                    return false;
                }
            }
            // Target filename does not exist anymore. Do a rename
            // change name in project file first (no actual file renaming on disk)
            //save old file, if it is opened;
            // remove old file from monitor list
            mProject->fileSystemWatcher()->removePath(oldName);

            if (!QFile::rename(oldName,newName)) {
                QMessageBox::critical(nullptr,
                                      tr("Rename failed"),
                                      tr("Failed to rename file '%1' to '%2'")
                                      .arg(oldName,newName),
                                      QMessageBox::Ok);
                return false;
            }
            mProject->renameUnit(unit,newName);

            // Add new filename to file minitor
            mProject->fileSystemWatcher()->addPath(newName);

            mProject->saveAll();

            return true;
        } else {
            //change folder name
            QString newName = value.toString().trimmed();
            if (newName.isEmpty())
                return false;
            if (newName ==  node->text)
                return false;
            mProject->renameFolderNode(node,newName);

            emit dataChanged(index,index);

            mProject->saveAll();
            return true;
        }

    }
    return false;
}

void ProjectModel::refreshIcon(const QModelIndex &index, bool update)
{
    if (!index.isValid())
        return;
    if (update)
        mIconProvider->update();
    QVector<int> roles;
    roles.append(Qt::DecorationRole);
    emit dataChanged(index,index, roles);
}

void ProjectModel::refreshIcon(const QString &filename)
{
    PProjectUnit unit=mProject->findUnit(filename);
    if (!unit)
        return;
    PProjectModelNode node=unit->node();
    QModelIndex index = getNodeIndex(node.get());
    refreshIcon(index);
}

void ProjectModel::refreshIcons()
{
    mIconProvider->update();
    mProject->rootNode();
}

void ProjectModel::refreshNodeIconRecursive(PProjectModelNode node)
{
    QModelIndex index=getNodeIndex(node.get());
    refreshIcon(index,false);
    foreach( PProjectModelNode child, node->children) {
        refreshNodeIconRecursive(child);
    }
}

QModelIndex ProjectModel::getNodeIndex(ProjectModelNode *node) const
{
    if (!node)
        return QModelIndex();
    PProjectModelNode parent = node->parent.lock();
    if (!parent) // root node
        return createIndex(0,0,node);
    int row = -1;
    for (int i=0;i<parent->children.count();i++) {
        const PProjectModelNode& pNode=parent->children[i];
        if (pNode.get()==node) {
            row = i;
        }
    }
    if (row<0)
        return QModelIndex();
    return createIndex(row,0,node);
}

QModelIndex ProjectModel::getParentIndex(ProjectModelNode * node) const
{
    PProjectModelNode parent = node->parent.lock();
    if (!parent) // root node
        return QModelIndex();
    PProjectModelNode grand = parent->parent.lock();
    if (!grand) {
        return createIndex(0,0,parent.get());
    }

    int row = grand->children.indexOf(parent);
    if (row<0)
        return QModelIndex();
    return createIndex(row,0,parent.get());
}

QModelIndex ProjectModel::rootIndex() const
{
    return getNodeIndex(mProject->rootNode().get());
}

bool ProjectModel::canDropMimeData(const QMimeData * data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent) const
{

    if (!data || action != Qt::MoveAction)
        return false;
    if (!parent.isValid())
        return false;
    // check if the format is supported
    QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;

    QModelIndex idx = parent;
//    if (row >= rowCount(parent) || row < 0) {
//        return false;
//    } else {
//        idx= index(row,column,parent);
//    }
    ProjectModelNode* p= static_cast<ProjectModelNode*>(idx.internalPointer());
    PProjectModelNode node = mProject->pointerToNode(p);
    if (node->isUnit)
        return false;
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        qint32 r, c;
        quintptr v;
        stream >> r >> c >> v;
        ProjectModelNode* droppedPointer= (ProjectModelNode*)(v);
        PProjectModelNode droppedNode = mProject->pointerToNode(droppedPointer);
        PProjectModelNode oldParent = droppedNode->parent.lock();
        if (oldParent == node)
            return false;
    }
    return true;
}

Qt::DropActions ProjectModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool ProjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent)
{
    // check if the action is supported
    if (!data || action != Qt::MoveAction)
        return false;
    // check if the format is supported
    QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;

    if (!parent.isValid())
        return false;
    ProjectModelNode* p= static_cast<ProjectModelNode*>(parent.internalPointer());
    PProjectModelNode node = mProject->pointerToNode(p);

    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QVector<int> rows,cols;
    QVector<intptr_t> pointers;
    while (!stream.atEnd()) {
        qint32 r, c;
        quintptr v;
        stream >> r >> c >> v;
        rows.append(r);
        cols.append(c);
        pointers.append(v);
    }
    for (int i=pointers.count()-1;i>=0;i--) {
        int r = rows[i];
        intptr_t v = pointers[i];
        ProjectModelNode* droppedPointer= (ProjectModelNode*)(v);
        PProjectModelNode droppedNode = mProject->pointerToNode(droppedPointer);
        PProjectModelNode oldParent = droppedNode->parent.lock();
        if (oldParent) {
            QModelIndex oldParentIndex = getNodeIndex(oldParent.get());
            beginRemoveRows(oldParentIndex,r,r);
            oldParent->children.removeAt(r);
            endRemoveRows();
        }
        QModelIndex newParentIndex = getNodeIndex(node.get());
        beginInsertRows(newParentIndex,node->children.count(),node->children.count());
        droppedNode->parent = node;
        node->children.append(droppedNode);
        if (droppedNode->isUnit) {
            PProjectUnit unit = droppedNode->pUnit.lock();
            unit->setFolder(mProject->getNodePath(node));
        }
        endInsertRows();
        mProject->saveAll();
        return true;
    }

    return false;
}

QMimeData *ProjectModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0)
        return nullptr;
    QStringList types = mimeTypes();
    if (types.isEmpty())
        return nullptr;
    QMimeData *data = new QMimeData();
    QString format = types.at(0);
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    QModelIndexList::ConstIterator it = indexes.begin();
    QList<QUrl> urls;
    for (; it != indexes.end(); ++it) {
        stream << (qint32)((*it).row()) << (qint32)((*it).column()) << (quintptr)((*it).internalPointer());
        ProjectModelNode* p = static_cast<ProjectModelNode*>((*it).internalPointer());
        if (p && p->isUnit) {
            PProjectUnit unit = p->pUnit.lock();
            if (unit)
                urls.append(QUrl::fromLocalFile(unit->fileName()));
        }
    }
    if (!urls.isEmpty())
        data->setUrls(urls);
    data->setData(format, encoded);
    return data;
}

ProjectModelSortFilterProxy::ProjectModelSortFilterProxy(QObject *parent):
    QSortFilterProxyModel(parent)
{

}

bool ProjectModelSortFilterProxy::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (!sourceModel())
        return false;
    ProjectModelNode* pLeft=nullptr;
    if (source_left.isValid())
        pLeft = static_cast<ProjectModelNode*>(source_left.internalPointer());
    ProjectModelNode* pRight=nullptr;
    if (source_right.isValid())
        pRight = static_cast<ProjectModelNode*>(source_right.internalPointer());
    if (!pLeft)
        return true;
    if (!pRight)
        return false;
    if (!pLeft->isUnit && pRight->isUnit)
        return true;
    if (pLeft->isUnit && !pRight->isUnit)
        return false;
    if (pLeft->priority!=pRight->priority)
        return pLeft->priority>pRight->priority;
    return QString::compare(pLeft->text, pRight->text)<0;
}
