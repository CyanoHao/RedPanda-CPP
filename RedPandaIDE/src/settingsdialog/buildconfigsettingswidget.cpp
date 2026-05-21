#include "buildconfigsettingswidget.h"
#include "ui_buildconfigsettingswidget.h"
#include "../settings/buildconfigmanager.h"
#include "../compiler/compilerinfo.h"
#include "../systemconsts.h"
#include "../widgets/compileargumentswidget.h"
#include "qt_utils/charsetinfo.h"
#include <QInputDialog>
#include <QMessageBox>

static CompilerType compilerTypeFromIndex(int idx)
{
    switch (idx) {
    case 0: return CompilerType::GCC;
    case 1: return CompilerType::AppleClang;
#ifdef ENABLE_SDCC
    case 2: return CompilerType::SDCC;
#endif
    default: return CompilerType::Unknown;
    }
}

static int indexFromCompilerType(CompilerType t)
{
    switch (t) {
    case CompilerType::Clang: /* fallthrough - share GCC family */
    case CompilerType::GCC:   return 0;
    case CompilerType::AppleClang: return 1;
#ifdef ENABLE_SDCC
    case CompilerType::SDCC:  return 2;
#endif
    default: return 0;
    }
}

static int linkModelToIndex(LinkModel m)
{
    switch (m) {
    case LinkModel::Dynamic:    return 0;
    case LinkModel::Static:     return 1;
    case LinkModel::PIE:        return 2;
    case LinkModel::StaticPIE:  return 3;
    default: return 0;
    }
}

static LinkModel indexToLinkModel(int idx)
{
    switch (idx) {
    case 1: return LinkModel::Static;
    case 2: return LinkModel::PIE;
    case 3: return LinkModel::StaticPIE;
    default: return LinkModel::Dynamic;
    }
}

BuildConfigSettingsWidget::BuildConfigSettingsWidget(const QString& name, const QString& group,
                                                       IconsManager *iconsManager, QWidget *parent)
    : SettingsWidget(name, group, iconsManager, parent)
    , ui(new Ui::BuildConfigSettingsWidget)
{
    ui->setupUi(this);
}

BuildConfigSettingsWidget::~BuildConfigSettingsWidget()
{
    delete ui;
}

void BuildConfigSettingsWidget::init()
{
    // Populate compiler family combobox
    ui->cbCompilerFamily->addItem(tr("GCC / LLVM Clang"));
    ui->cbCompilerFamily->addItem(tr("Apple Clang"));
#ifdef ENABLE_SDCC
    ui->cbCompilerFamily->addItem(tr("SDCC"));
#endif

    // Populate link model combobox
    ui->cmbLinkModel->addItem(tr("Dynamic"));
    ui->cmbLinkModel->addItem(tr("Static"));
    ui->cmbLinkModel->addItem(tr("PIE"));
    ui->cmbLinkModel->addItem(tr("Static-PIE"));

    // Populate encoding combobox
    ui->cbEncodingDetails->setVisible(false);
    ui->cbEncoding->addItem(tr("System Default (%1)")
        .arg(QString(pCharsetInfoManager->getDefaultSystemEncoding())),
        ENCODING_SYSTEM_DEFAULT);
#ifdef Q_OS_WIN
    ui->cbEncoding->addItem(tr("System OEM (%1)")
        .arg(QString(pCharsetInfoManager->getDefaultConsoleEncoding())),
        ENCODING_OEM_DEFAULT);
#endif
    ui->cbEncoding->addItem(tr("UTF-8"), ENCODING_UTF8);
    foreach (const QString& langName, pCharsetInfoManager->languageNames()) {
        ui->cbEncoding->addItem(langName, langName);
    }

    SettingsWidget::init();
}

CompilerType BuildConfigSettingsWidget::currentFamily() const
{
    return compilerTypeFromIndex(ui->cbCompilerFamily->currentIndex());
}

void BuildConfigSettingsWidget::doLoad()
{
    BuildConfigManager& bm = pSettings->buildConfigManager();

    // Set family combobox to the first family that has configs, or GCC
    CompilerType family = CompilerType::GCC;
    if (!bm.configsFor(CompilerType::GCC).isEmpty())
        family = CompilerType::GCC;
    else if (!bm.configsFor(CompilerType::AppleClang).isEmpty())
        family = CompilerType::AppleClang;
#ifdef ENABLE_SDCC
    else if (!bm.configsFor(CompilerType::SDCC).isEmpty())
        family = CompilerType::SDCC;
#endif

    ui->cbCompilerFamily->blockSignals(true);
    ui->cbCompilerFamily->setCurrentIndex(indexFromCompilerType(family));
    ui->cbCompilerFamily->blockSignals(false);

    // Populate build config combobox
    ui->cbBuildConfig->blockSignals(true);
    ui->cbBuildConfig->clear();
    QList<BuildConfiguration> configs = bm.configsFor(family);
    for (const auto& cfg : configs) {
        ui->cbBuildConfig->addItem(cfg.name);
    }

    ui->btnRename->setEnabled(!configs.isEmpty());
    ui->btnRemove->setEnabled(!configs.isEmpty());

    if (!configs.isEmpty()) {
        QString activeName = bm.activeConfigName();
        int idx = 0;
        if (!activeName.isEmpty()) {
            for (int i = 0; i < configs.size(); i++) {
                if (configs[i].name == activeName) { idx = i; break; }
            }
        }
        ui->cbBuildConfig->setCurrentIndex(idx);
    }
    ui->cbBuildConfig->blockSignals(false);

    loadCurrentConfig();
}

void BuildConfigSettingsWidget::doSave()
{
    BuildConfigManager& bm = pSettings->buildConfigManager();
    CompilerType family = currentFamily();
    QString name = ui->cbBuildConfig->currentText();
    if (!name.isEmpty()) {
        saveCurrentConfig();
        bm.setActiveConfig(name);
    }
    bm.save(pSettings->dirs().config(DirSettings::DataType::None) + "/build_configs.json");
}

void BuildConfigSettingsWidget::loadCurrentConfig()
{
    BuildConfigManager& bm = pSettings->buildConfigManager();
    CompilerType family = currentFamily();
    int idx = ui->cbBuildConfig->currentIndex();
    QList<BuildConfiguration> configs = bm.configsFor(family);
    if (idx < 0 || idx >= configs.size())
        return;

    const BuildConfiguration& cfg = configs[idx];

    // Link model
    LinkModel lm = cfg.linkModelOverride.value_or(LinkModel::Dynamic);
    ui->cmbLinkModel->setCurrentIndex(linkModelToIndex(lm));

    // Charset
    ui->chkAutoAddCharset->setChecked(cfg.autoAddCharsetParams);
    // encoding dropdown
    QByteArray execEncoding = cfg.execCharset.toUtf8();
    if (execEncoding.isEmpty()) execEncoding = ENCODING_SYSTEM_DEFAULT;
    if (execEncoding == ENCODING_AUTO_DETECT
        || execEncoding == ENCODING_SYSTEM_DEFAULT
        || execEncoding == ENCODING_OEM_DEFAULT
        || execEncoding == ENCODING_UTF8) {
        int eidx = ui->cbEncoding->findData(execEncoding);
        ui->cbEncoding->setCurrentIndex(eidx);
        ui->cbEncodingDetails->clear();
        ui->cbEncodingDetails->setVisible(false);
    } else {
        QString encoding = execEncoding;
        QString language = pCharsetInfoManager->findLanguageByCharsetName(encoding);
        ui->cbEncoding->setCurrentText(language);
        ui->cbEncodingDetails->setVisible(true);
        ui->cbEncodingDetails->clear();
        QList<PCharsetInfo> infos = pCharsetInfoManager->findCharsetsByLanguageName(language);
        foreach (const PCharsetInfo& info, infos) {
            ui->cbEncodingDetails->addItem(info->name);
        }
        ui->cbEncodingDetails->setCurrentText(encoding);
    }

    // Misc
    ui->chkForceEnglishOutput->setChecked(false); // not in BuildConfig currently

    // Custom params
    ui->chkUseCustomCompilerParams->setChecked(!cfg.customCompileParams.isEmpty());
    ui->txtCustomCompileParams->setPlainText(cfg.customCompileParams);
    ui->txtCustomCompileParams->setEnabled(!cfg.customCompileParams.isEmpty());
    ui->chkUseCustomLinkParams->setChecked(!cfg.customLinkParams.isEmpty());
    ui->txtCustomLinkParams->setPlainText(cfg.customLinkParams);
    ui->txtCustomLinkParams->setEnabled(!cfg.customLinkParams.isEmpty());
    ui->txtCustomLinkParams->setPlainText(cfg.customLinkParams);

    // Compiler options (build-config-level)
    CompilerType ct = (family == CompilerType::AppleClang) ? CompilerType::AppleClang : family;
    ui->optionTabs->resetUI(ct, cfg.compilerOptions,
                             CompileArgumentsWidget::OptionLevelFilter::BuildConfig);
}

void BuildConfigSettingsWidget::saveCurrentConfig()
{
    BuildConfigManager& bm = pSettings->buildConfigManager();
    CompilerType family = currentFamily();
    QString name = ui->cbBuildConfig->currentText();
    QList<BuildConfiguration> configs = bm.configsFor(family);
    int idx = ui->cbBuildConfig->currentIndex();
    if (idx < 0 || idx >= configs.size()) return;

    BuildConfiguration cfg = configs[idx];

    cfg.linkModelOverride = indexToLinkModel(ui->cmbLinkModel->currentIndex());

    cfg.autoAddCharsetParams = ui->chkAutoAddCharset->isChecked();
    if (ui->cbEncodingDetails->isVisible()) {
        cfg.execCharset = ui->cbEncodingDetails->currentText();
    } else {
        cfg.execCharset = ui->cbEncoding->currentData().toString();
    }

    cfg.customCompileParams = ui->chkUseCustomCompilerParams->isChecked()
        ? ui->txtCustomCompileParams->toPlainText().trimmed() : QString();
    cfg.customLinkParams = ui->chkUseCustomLinkParams->isChecked()
        ? ui->txtCustomLinkParams->toPlainText().trimmed() : QString();

    cfg.compilerOptions = ui->optionTabs->arguments(false);

    bm.updateConfig(family, cfg.name, cfg);
}

// ---- Slots ----

void BuildConfigSettingsWidget::on_cbCompilerFamily_currentIndexChanged(int /*index*/)
{
    BuildConfigManager& bm = pSettings->buildConfigManager();
    CompilerType family = currentFamily();

    ui->cbBuildConfig->blockSignals(true);
    ui->cbBuildConfig->clear();
    QList<BuildConfiguration> configs = bm.configsFor(family);
    for (const auto& cfg : configs) {
        ui->cbBuildConfig->addItem(cfg.name);
    }
    ui->btnRename->setEnabled(!configs.isEmpty());
    ui->btnRemove->setEnabled(!configs.isEmpty());
    if (!configs.isEmpty())
        ui->cbBuildConfig->setCurrentIndex(0);
    ui->cbBuildConfig->blockSignals(false);
    loadCurrentConfig();
}

void BuildConfigSettingsWidget::on_cbBuildConfig_currentIndexChanged(int /*index*/)
{
    saveCurrentConfig();
    loadCurrentConfig();
}

void BuildConfigSettingsWidget::on_btnAdd_clicked()
{
    QString name = QInputDialog::getText(this, tr("New Build Config"), tr("Name"));
    name = name.trimmed();
    if (name.isEmpty()) return;

    BuildConfiguration cfg;
    cfg.name = name;
    cfg.compilerTypeFamily = currentFamily();
    pSettings->buildConfigManager().addConfig(currentFamily(), cfg);
    setSettingsChanged();
    doLoad();
}

void BuildConfigSettingsWidget::on_btnCopy_clicked()
{
    CompilerType family = currentFamily();
    QList<BuildConfiguration> configs = pSettings->buildConfigManager().configsFor(family);
    int idx = ui->cbBuildConfig->currentIndex();
    if (idx < 0 || idx >= configs.size()) return;

    QString name = QInputDialog::getText(this, tr("Copy Build Config"),
        tr("New name"), QLineEdit::Normal,
        tr("%1 Copy").arg(configs[idx].name));
    name = name.trimmed();
    if (name.isEmpty()) return;

    BuildConfiguration cfg = configs[idx];
    cfg.name = name;
    pSettings->buildConfigManager().addConfig(family, cfg);
    setSettingsChanged();
    doLoad();
}

void BuildConfigSettingsWidget::on_btnRename_clicked()
{
    CompilerType family = currentFamily();
    QList<BuildConfiguration> configs = pSettings->buildConfigManager().configsFor(family);
    int idx = ui->cbBuildConfig->currentIndex();
    if (idx < 0 || idx >= configs.size()) return;

    QString name = QInputDialog::getText(this, tr("Rename Build Config"),
        tr("New name"), QLineEdit::Normal, configs[idx].name);
    name = name.trimmed();
    if (name.isEmpty()) return;

    BuildConfiguration cfg = configs[idx];
    pSettings->buildConfigManager().updateConfig(family, cfg.name, cfg);
    pSettings->buildConfigManager().updateConfig(family, cfg.name, {});
    // Actually we need to remove the old and add the new one
    pSettings->buildConfigManager().removeConfig(family, cfg.name);
    cfg.name = name;
    pSettings->buildConfigManager().addConfig(family, cfg);
    setSettingsChanged();
    doLoad();
}

void BuildConfigSettingsWidget::on_btnRemove_clicked()
{
    CompilerType family = currentFamily();
    QList<BuildConfiguration> configs = pSettings->buildConfigManager().configsFor(family);
    int idx = ui->cbBuildConfig->currentIndex();
    if (idx < 0 || idx >= configs.size()) return;

    if (QMessageBox::question(this, tr("Remove"),
            tr("Do you really want to remove \"%1\"?").arg(configs[idx].name),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)
        == QMessageBox::Yes) {
        pSettings->buildConfigManager().removeConfig(family, configs[idx].name);
        setSettingsChanged();
        doLoad();
    }
}

void BuildConfigSettingsWidget::on_cbEncoding_currentTextChanged(const QString& arg1)
{
    ui->cbEncodingDetails->clear();
    if (arg1 == tr("System Default (%1)")
            .arg(QString(pCharsetInfoManager->getDefaultSystemEncoding()))
#ifdef Q_OS_WIN
        || arg1 == tr("System OEM (%1)")
            .arg(QString(pCharsetInfoManager->getDefaultConsoleEncoding()))
#endif
        || arg1 == tr("UTF-8")
        || arg1.isEmpty()) {
        ui->cbEncodingDetails->setVisible(false);
    } else {
        QList<PCharsetInfo> infos = pCharsetInfoManager->findCharsetsByLanguageName(arg1);
        foreach (const PCharsetInfo& info, infos) {
            ui->cbEncodingDetails->addItem(info->name);
        }
        ui->cbEncodingDetails->setVisible(infos.size() > 0);
    }
}

void BuildConfigSettingsWidget::on_cbEncodingDetails_currentTextChanged(const QString& /*arg1*/)
{
    // handled in save
}

void BuildConfigSettingsWidget::updateIcons(const QSize& /*size*/)
{
}
