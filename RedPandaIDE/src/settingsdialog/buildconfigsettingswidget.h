#ifndef BUILDCONFIGSETTINGSWIDGET_H
#define BUILDCONFIGSETTINGSWIDGET_H

#include "settingswidget.h"
#include "../settings/buildconfig.h"

namespace Ui {
class BuildConfigSettingsWidget;
}

class BuildConfigSettingsWidget : public SettingsWidget
{
    Q_OBJECT
public:
    explicit BuildConfigSettingsWidget(const QString& name, const QString& group,
                                        IconsManager *iconsManager, QWidget *parent = nullptr);
    ~BuildConfigSettingsWidget();

    void init() override;
protected:
    void doLoad() override;
    void doSave() override;
    void updateIcons(const QSize& size) override;

private:
    void loadCurrentConfig();
    void saveCurrentConfig();
    CompilerType currentFamily() const;

    Ui::BuildConfigSettingsWidget *ui;

private slots:
    void on_cbCompilerFamily_currentIndexChanged(int index);
    void on_cbBuildConfig_currentIndexChanged(int index);
    void on_btnAdd_clicked();
    void on_btnCopy_clicked();
    void on_btnRename_clicked();
    void on_btnRemove_clicked();
    void on_cbEncoding_currentTextChanged(const QString& arg1);
    void on_cbEncodingDetails_currentTextChanged(const QString& arg1);
};

#endif // BUILDCONFIGSETTINGSWIDGET_H
