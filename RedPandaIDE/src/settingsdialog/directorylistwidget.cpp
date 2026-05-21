#include "directorylistwidget.h"
#include "ui_directorylistwidget.h"
#include "../iconsmanager.h"

#include <QFileDialog>
#include <QStringListModel>
#include <QDebug>

DirectoryListWidget::DirectoryListWidget(IconsManager *iconsManager,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectoryListWidget)
{
    ui->setupUi(this);
    mIconsManager = iconsManager;
    QItemSelectionModel *m=ui->listView->selectionModel();
    ui->listView->setModel(&mModel);
    delete m;
    connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &DirectoryListWidget::selectionChanged);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    onUpdateIcons();
}

DirectoryListWidget::~DirectoryListWidget()
{
    delete ui;
}

void DirectoryListWidget::setDirList(const QStringList &list)
{
    mModel.setStringList(list);
    QModelIndexList lst =ui->listView->selectionModel()->selectedIndexes();
    ui->btnDelete->setEnabled(lst.count()>0);
}

QStringList DirectoryListWidget::dirList() const
{
    return mModel.stringList();
}

Qt::ItemFlags DirectoryListWidget::ListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::NoItemFlags;
    if (index.isValid()) {
        flags = Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable ;
    } else if (index.row() == -1) {
        flags = Qt::ItemIsDropEnabled;
    }
    return flags;
}

void DirectoryListWidget::on_btnAdd_pressed()
{
    QString folder = QFileDialog::getExistingDirectory(this,tr("Choose Folder"));
    if (!folder.isEmpty()) {
        int row = mModel.rowCount();
        mModel.insertRow(row);
        QModelIndex index= mModel.index(row,0);
        mModel.setData(index,folder,Qt::DisplayRole);
    }
}

void DirectoryListWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
    ui->btnDelete->setEnabled(!selected.isEmpty());
}

void DirectoryListWidget::on_btnDelete_pressed()
{
    QModelIndexList lst =ui->listView->selectionModel()->selectedIndexes();
    if (lst.count()>0) {
        mModel.removeRow(lst[0].row());
    }
}

void DirectoryListWidget::on_btnRemoveInvalid_pressed()
{
    QStringList lst;
    foreach (const QString& folder, dirList() ) {
        QFileInfo info(folder);
        if (info.exists() && info.isDir() ) {
            lst.append(folder.trimmed());
        }
    }
    setDirList(lst);
}

void DirectoryListWidget::onUpdateIcons()
{
    mIconsManager->setIcon(ui->btnAdd,IconsManager::ACTION_MISC_ADD);
    mIconsManager->setIcon(ui->btnDelete, IconsManager::ACTION_MISC_REMOVE);
    mIconsManager->setIcon(ui->btnRemoveInvalid, IconsManager::ACTION_MISC_VALIDATE);
}
