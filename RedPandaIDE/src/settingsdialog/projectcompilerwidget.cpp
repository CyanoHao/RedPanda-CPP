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
#include "projectcompilerwidget.h"
#include "ui_projectcompilerwidget.h"
#include "../settings.h"
#include "../project.h"
#include "../mainwindow.h"
#include <qt_utils/charsetinfo.h>
#include <QMessageBox>

ProjectCompilerWidget::ProjectCompilerWidget(const QString &name, const QString& group, IconsManager *iconsManager, QWidget *parent) :
    SettingsWidget(name,group,iconsManager,parent),
    ui(new Ui::ProjectCompilerWidget)
{
    ui->setupUi(this);
}

ProjectCompilerWidget::~ProjectCompilerWidget()
{
    delete ui;
}

PToolchain ProjectCompilerWidget::currentToolchain() const
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0)
        return nullptr;
    QString tid = ui->cbToolchain->currentData().toString();
    return pSettings->toolchainManager().findById(tid);
}

void ProjectCompilerWidget::populateBuildConfigCombo(const PToolchain& tc)
{
    ui->cbBuildConfig->blockSignals(true);
    ui->cbBuildConfig->clear();
    if (tc) {
        QList<BuildConfiguration> configs = pSettings->buildConfigManager().configsFor(tc->compilerType);
        for (const BuildConfiguration& cfg : configs) {
            ui->cbBuildConfig->addItem(cfg.name);
        }
        // Select current build config
        QString currentName = pMainWindow->project()->options().buildConfigName;
        if (!currentName.isEmpty()) {
            int idx = ui->cbBuildConfig->findText(currentName);
            if (idx >= 0)
                ui->cbBuildConfig->setCurrentIndex(idx);
        } else {
            // Select active config
            QString activeName = pSettings->buildConfigManager().activeConfigName();
            if (!activeName.isEmpty()) {
                int idx = ui->cbBuildConfig->findText(activeName);
                if (idx >= 0)
                    ui->cbBuildConfig->setCurrentIndex(idx);
            }
        }
    }
    ui->cbBuildConfig->blockSignals(false);
}

void ProjectCompilerWidget::refreshOptions()
{
    PToolchain tc = currentToolchain();
    if (!tc)
        return;

    ui->panelAddCharset->setVisible(tc->compilerType != CompilerType::Clang);

    // Merge toolchain and build config options
    QMap<QString, QString> mergedOptions = tc->compilerOptions;
    QString bcName = ui->cbBuildConfig->currentText();
    if (!bcName.isEmpty()) {
        QList<BuildConfiguration> configs = pSettings->buildConfigManager().configsFor(tc->compilerType);
        for (const BuildConfiguration& cfg : configs) {
            if (cfg.name == bcName) {
                for (auto it = cfg.compilerOptions.begin(); it != cfg.compilerOptions.end(); ++it)
                    mergedOptions[it.key()] = it.value();
                break;
            }
        }
    }

    ui->tabOptions->resetUI(tc->compilerType, mergedOptions);

    // Map linkModel to staticLink
    PBuildConfiguration cfg = pMainWindow->project()->resolveBuildConfig();
    if (cfg) {
        LinkModel model = cfg->linkModelOverride.value_or(tc->defaultLinkModel);
        mStaticLink = (model == LinkModel::Static || model == LinkModel::StaticPIE);
    } else {
        mStaticLink = (tc->defaultLinkModel == LinkModel::Static || tc->defaultLinkModel == LinkModel::StaticPIE);
    }
    ui->chkStaticLink->setChecked(mStaticLink);

    ui->chkAddCharset->setChecked(mAddCharset);

    QByteArray execEncoding = mExecCharset;
    if (execEncoding == ENCODING_AUTO_DETECT
            || execEncoding == ENCODING_SYSTEM_DEFAULT
            || execEncoding == ENCODING_OEM_DEFAULT
            || execEncoding == ENCODING_UTF8) {
        int index =ui->cbEncoding->findData(execEncoding);
        ui->cbEncoding->setCurrentIndex(index);
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
}

void ProjectCompilerWidget::doLoad()
{
    std::shared_ptr<Project> project = pMainWindow->project();
    mOptions = project->options().compilerOptions;

    // Load toolchain selection
    QString tid = project->options().toolchainId;
    if (!tid.isEmpty()) {
        int idx = ui->cbToolchain->findData(tid);
        if (idx >= 0)
            ui->cbToolchain->setCurrentIndex(idx);
    }

    // Populate build config combo
    PToolchain tc = currentToolchain();
    populateBuildConfigCombo(tc);

    // Load staticLink, charset settings
    PBuildConfiguration cfg = project->resolveBuildConfig();
    if (cfg && tc) {
        LinkModel model = cfg->linkModelOverride.value_or(tc->defaultLinkModel);
        mStaticLink = (model == LinkModel::Static || model == LinkModel::StaticPIE);
        mAddCharset = cfg->autoAddCharsetParams;
        mExecCharset = cfg->execCharset.toUtf8();
    } else {
        mStaticLink = false;
        mAddCharset = true;
        mExecCharset = ENCODING_SYSTEM_DEFAULT;
    }

    refreshOptions();
}

void ProjectCompilerWidget::doSave()
{
    PToolchain tc = currentToolchain();
    if (!tc)
        return;

    pMainWindow->project()->setToolchainId(tc->id);
    pMainWindow->project()->options().buildConfigName = ui->cbBuildConfig->currentText();
    pMainWindow->project()->options().compilerOptions = ui->tabOptions->arguments(true);
    if (tc->compilerType != CompilerType::Clang)
        pMainWindow->project()->options().addCharset = ui->chkAddCharset->isChecked();
    pMainWindow->project()->options().staticLink = ui->chkStaticLink->isChecked();

    if (ui->cbEncodingDetails->isVisible()) {
        pMainWindow->project()->options().execEncoding = ui->cbEncodingDetails->currentText().toLocal8Bit();
    } else {
        pMainWindow->project()->options().execEncoding = ui->cbEncoding->currentData().toString().toLocal8Bit();
    }
    mOptions = pMainWindow->project()->options().compilerOptions;
    mStaticLink = pMainWindow->project()->options().staticLink;
    mAddCharset = pMainWindow->project()->options().addCharset;
    mExecCharset = pMainWindow->project()->options().execEncoding;
    pMainWindow->project()->saveOptions();
}

void ProjectCompilerWidget::init()
{
    ui->cbToolchain->blockSignals(true);
    ui->cbToolchain->clear();
    for (const Toolchain& tc : pSettings->toolchainManager().toolchains()) {
        ui->cbToolchain->addItem(tc.name, tc.id);
    }
    ui->cbToolchain->blockSignals(false);
    ui->cbEncodingDetails->setVisible(false);
    ui->cbEncoding->clear();
    ui->cbEncoding->addItem(tr("System Default(%1)").arg(QString(pCharsetInfoManager->getDefaultSystemEncoding())),ENCODING_SYSTEM_DEFAULT);
#ifdef Q_OS_WIN
    ui->cbEncoding->addItem(tr("System OEM(%1)").arg(QString(pCharsetInfoManager->getDefaultConsoleEncoding())),ENCODING_OEM_DEFAULT);
#endif
    ui->cbEncoding->addItem(tr("UTF-8"),ENCODING_UTF8);
    foreach (const QString& langName, pCharsetInfoManager->languageNames()) {
        ui->cbEncoding->addItem(langName,langName);
    }
    SettingsWidget::init();
}

void ProjectCompilerWidget::on_cbToolchain_currentIndexChanged(int index)
{
    if (index<0)
        return;
    std::shared_ptr<Project> project = pMainWindow->project();
    clearSettingsChanged();
    disconnectInputs();
    ui->cbToolchain->blockSignals(true);
    auto action = finally([this]{
        ui->cbToolchain->blockSignals(false);
        populateBuildConfigCombo(currentToolchain());
        refreshOptions();
        connectInputs();
    });

    PToolchain tc = currentToolchain();
#ifdef ENABLE_SDCC
    if (tc) {
        if (project->options().type==ProjectType::MicroController) {
            if (tc->compilerType!=CompilerType::SDCC) {
                QMessageBox::information(this,
                                         tr("Wrong Compiler Type"),
                                         tr("Compiler %1 can't compile a microcontroller project.").arg(tc->name));
                ui->cbToolchain->setCurrentIndex(0); // Reset to first
                return;
            }
        } else {
            if (tc->compilerType==CompilerType::SDCC) {
                QMessageBox::information(this,
                                         tr("Wrong Compiler Type"),
                                         tr("Compiler %1 can only compile microcontroller project.").arg(tc->name));
                ui->cbToolchain->setCurrentIndex(0); // Reset to first
                return;
            }
        }
    }
#endif
    if (QMessageBox::warning(
                this,
                tr("Change Project Toolchain"),
                tr("Change the project's toolchain will reset all custom compiler options.")
                +"<br />"
                + tr("Do you really want to do that?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) != QMessageBox::Yes) {
        // Revert to original selection
        QString originalId = project->options().toolchainId;
        if (!originalId.isEmpty()) {
            int idx = ui->cbToolchain->findData(originalId);
            if (idx >= 0)
                ui->cbToolchain->setCurrentIndex(idx);
        }
        return;
    }

    // Reset options to toolchain defaults
    if (tc) {
        mOptions = tc->compilerOptions;
        LinkModel model = tc->defaultLinkModel;
        mStaticLink = (model == LinkModel::Static || model == LinkModel::StaticPIE);
        mAddCharset = true;
        mExecCharset = ENCODING_SYSTEM_DEFAULT;
    }

    setSettingsChanged();
}

void ProjectCompilerWidget::on_cbBuildConfig_currentIndexChanged(int index)
{
    if (index < 0)
        return;
    // Build config change resets options to that config's defaults
    refreshOptions();
    setSettingsChanged();
}

void ProjectCompilerWidget::on_cbEncoding_currentTextChanged(const QString &/*arg1*/)
{
    QString userData = ui->cbEncoding->currentData().toString();
    if (userData == ENCODING_AUTO_DETECT
            || userData == ENCODING_SYSTEM_DEFAULT
            || userData == ENCODING_OEM_DEFAULT
            || userData == ENCODING_UTF8) {
        ui->cbEncodingDetails->setVisible(false);
        ui->cbEncodingDetails->clear();
    } else {
        ui->cbEncodingDetails->setVisible(true);
        ui->cbEncodingDetails->clear();
        QList<PCharsetInfo> infos = pCharsetInfoManager->findCharsetsByLanguageName(userData);
        foreach (const PCharsetInfo& info, infos) {
            ui->cbEncodingDetails->addItem(info->name);
        }
    }
}


void ProjectCompilerWidget::on_cbEncodingDetails_currentTextChanged(const QString &/*arg1*/)
{

}
