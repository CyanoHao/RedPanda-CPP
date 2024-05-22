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

ProjectCompilerWidget::ProjectCompilerWidget(const QString &name, const QString &group, QWidget *parent) :
    SettingsWidget(name,group,parent),
    ui(new Ui::ProjectCompilerWidget)
{
    ui->setupUi(this);
}

ProjectCompilerWidget::~ProjectCompilerWidget()
{
    delete ui;
}

void ProjectCompilerWidget::refreshOptions()
{
    Settings::PCompilerSet pSet = pSettings->compilerSets().getSet(ui->cbCompilerSet->currentIndex());
    if (!pSet)
        return;
    ui->panelAddCharset->setVisible(pSet->compilerType()!=CompilerType::Clang);
    //ui->chkAddCharset->setEnabled(pSet->compilerType()!=COMPILER_CLANG);

    ui->tabOptions->resetUI(pSet,mOptions);

    ui->chkStaticLink->setChecked(mStaticLink);
    ui->chkAddCharset->setChecked(mAddCharset);

    QByteArray execEncoding = mExecCharset;
    int index =ui->cbEncoding->findData(execEncoding);
    ui->cbEncoding->setCurrentIndex(index);
}

void ProjectCompilerWidget::doLoad()
{
    mOptions = pMainWindow->project()->options().compilerOptions;
    Settings::PCompilerSet pSet = pSettings->compilerSets().getSet(ui->cbCompilerSet->currentIndex());
    if (mOptions.isEmpty() && pSet)
        mOptions = pSet->compileOptions();
    mStaticLink = pMainWindow->project()->options().staticLink;
    mAddCharset = pMainWindow->project()->options().addCharset;
    mExecCharset = pMainWindow->project()->options().execEncoding;
    ui->cbCompilerSet->blockSignals(true);
    ui->cbCompilerSet->setCurrentIndex(pMainWindow->project()->options().compilerSet);
    ui->cbCompilerSet->blockSignals(false);
    refreshOptions();
}

void ProjectCompilerWidget::doSave()
{
    Settings::PCompilerSet pSet = pSettings->compilerSets().getSet(ui->cbCompilerSet->currentIndex());
    if (!pSet)
        return;
    pMainWindow->project()->setCompilerSet(ui->cbCompilerSet->currentIndex());
    pMainWindow->project()->options().compilerOptions = ui->tabOptions->arguments(true);
    if (pSet->compilerType()!=CompilerType::Clang)
        pMainWindow->project()->options().addCharset = ui->chkAddCharset->isChecked();
    pMainWindow->project()->options().staticLink = ui->chkStaticLink->isChecked();

    pMainWindow->project()->options().execEncoding = ui->cbEncoding->currentData().toString().toLatin1();

    mOptions = pMainWindow->project()->options().compilerOptions;
    mStaticLink = pMainWindow->project()->options().staticLink;
    mAddCharset = pMainWindow->project()->options().addCharset;
    mExecCharset = pMainWindow->project()->options().execEncoding;
    pMainWindow->project()->saveOptions();
}

void ProjectCompilerWidget::init()
{
    ui->cbCompilerSet->blockSignals(true);
    ui->cbCompilerSet->clear();
    for (size_t i=0;i<pSettings->compilerSets().size();i++) {
        ui->cbCompilerSet->addItem(pSettings->compilerSets().getSet(i)->name());
    }
    ui->cbCompilerSet->blockSignals(false);
    ui->cbEncoding->clear();
    for (const Win32AnsiCodePage &item : Win32AnsiCodePage::codePageList) {
        ui->cbEncoding->addItem(item.displayName, item.encoding);
    }
    SettingsWidget::init();
}

void ProjectCompilerWidget::on_cbCompilerSet_currentIndexChanged(int index)
{
    if (index<0)
        return;
    std::shared_ptr<Project> project = pMainWindow->project();
    clearSettingsChanged();
    disconnectInputs();
    ui->cbCompilerSet->blockSignals(true);
    auto action = finally([this]{
        ui->cbCompilerSet->blockSignals(false);
        refreshOptions();
        connectInputs();
    });
    Settings::PCompilerSet pSet=pSettings->compilerSets().getSet(index);
#ifdef ENABLE_SDCC
    if (pSet) {
        if (project->options().type==ProjectType::MicroController) {
            if (pSet->compilerType()!=CompilerType::SDCC) {
                QMessageBox::information(this,
                                         tr("Wrong Compiler Type"),
                                         tr("Compiler %1 can't compile a microcontroller project.").arg(pSet->name())
                                         );
                ui->cbCompilerSet->setCurrentIndex(project->options().compilerSet);
                return;
            }
        } else {
            if (pSet->compilerType()==CompilerType::SDCC) {
                QMessageBox::information(this,
                                         tr("Wrong Compiler Type"),
                                         tr("Compiler %1 can only compile microcontroller project.").arg(pSet->name())
                                         );
                ui->cbCompilerSet->setCurrentIndex(project->options().compilerSet);
                return;
            }
        }
    }
#endif
    if (QMessageBox::warning(
                this,
                tr("Change Project Compiler Set"),
                tr("Change the project's compiler set will lose all custom compiler set options.")
                +"<br />"
                + tr("Do you really want to do that?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) != QMessageBox::Yes) {
        ui->cbCompilerSet->setCurrentIndex(project->options().compilerSet);
        return;
    }
    mOptions = pSet->compileOptions();
    mStaticLink = pSet->staticLink();
    mAddCharset = pSet->autoAddCharsetParams();
    mExecCharset = pSet->execCharset().toUtf8();

    setSettingsChanged();
    //project->saveOptions();
}
