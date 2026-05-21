#ifndef DIRECTORYLISTWIDGET_H
#define DIRECTORYLISTWIDGET_H

#include <QWidget>
#include <QStringListModel>

namespace Ui {
class DirectoryListWidget;
}

class QItemSelection;
class IconsManager;

class DirectoryListWidget : public QWidget
{
    Q_OBJECT
    class ListModel: public QStringListModel {
    public:
       Qt::ItemFlags flags(const QModelIndex &index) const;
    };

public:
    explicit DirectoryListWidget(IconsManager *iconsManager,QWidget *parent = nullptr);
    ~DirectoryListWidget();

    void setDirList(const QStringList& list);
    QStringList dirList() const;

private slots:
    void on_btnDelete_pressed();

    void on_btnAdd_pressed();

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_btnRemoveInvalid_pressed();

    void onUpdateIcons();

private:
    Ui::DirectoryListWidget *ui;
    ListModel mModel;
    IconsManager *mIconsManager;
};

#endif // DIRECTORYLISTWIDGET_H
