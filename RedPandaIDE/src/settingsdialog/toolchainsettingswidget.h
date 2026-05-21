#ifndef TOOLCHAINSETTINGSWIDGET_H
#define TOOLCHAINSETTINGSWIDGET_H

#include "settingswidget.h"
#include "../settings/toolchain.h"
#include <QUuid>

namespace Ui {
class ToolchainSettingsWidget;
}

class DirectoryListWidget;

class ToolchainSettingsWidget : public SettingsWidget
{
    Q_OBJECT
public:
    explicit ToolchainSettingsWidget(const QString& name, const QString& group,
                                     IconsManager *iconsManager, QWidget *parent = nullptr);
    ~ToolchainSettingsWidget();

    void init() override;
protected:
    void doLoad() override;
    void doSave() override;
    void updateIcons(const QSize& size) override;

private:
    void loadCurrentToolchain();
    void saveCurrentToolchain();
    QString currentToolchainId() const;

    Ui::ToolchainSettingsWidget *ui;
    DirectoryListWidget* mBinDirWidget;
    DirectoryListWidget* mLibDirWidget;
    DirectoryListWidget* mCIncludeDirWidget;
    DirectoryListWidget* mCppIncludeDirWidget;

private slots:
    void on_cbToolchain_currentIndexChanged(int index);
    void on_btnFindCompilers_clicked();
    void on_btnAddBlank_clicked();
    void on_btnAddByFolder_clicked();
    void on_btnAddByFile_clicked();
    void on_btnCopy_clicked();
    void on_btnRename_clicked();
    void on_btnRemove_clicked();
    void on_btnChooseCCompiler_clicked();
    void on_btnChooseCppCompiler_clicked();
    void on_btnChooseMake_clicked();
    void on_btnChooseDebugger_clicked();
    void on_btnChooseGDBServer_clicked();
    void on_btnChooseResourceCompiler_clicked();
};

#endif // TOOLCHAINSETTINGSWIDGET_H
