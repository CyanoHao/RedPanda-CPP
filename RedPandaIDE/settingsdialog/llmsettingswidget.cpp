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
#include "llmsettingswidget.h"
#include "ui_llmsettingswidget.h"
#include "../settings.h"
#include "../mainwindow.h"

LLMSettingsWidget::LLMSettingsWidget(const QString& name, const QString& group, QWidget *parent) :
    SettingsWidget(name, group, parent),
    ui(new Ui::LLMSettingsWidget)
{
    ui->setupUi(this);
}

LLMSettingsWidget::~LLMSettingsWidget()
{
    delete ui;
}

void LLMSettingsWidget::doLoad()
{
    ui->chkEnabled->setChecked(pSettings->llm().enabled());
    ui->txtEndpoint->setText(pSettings->llm().endpoint());
    ui->txtApiKey->setText(pSettings->llm().apiKey());
    ui->txtModelName->setText(pSettings->llm().modelName());
    ui->spinTimeout->setValue(pSettings->llm().timeout());
}

void LLMSettingsWidget::doSave()
{
    pSettings->llm().setEnabled(ui->chkEnabled->isChecked());
    pSettings->llm().setEndpoint(ui->txtEndpoint->text().trimmed());
    pSettings->llm().setApiKey(ui->txtApiKey->text().trimmed());
    pSettings->llm().setModelName(ui->txtModelName->text().trimmed());
    pSettings->llm().setTimeout(ui->spinTimeout->value());
    pSettings->llm().save();
}

void LLMSettingsWidget::updateIcons(const QSize &/*size*/)
{
    // No icons to update for this widget
}

void LLMSettingsWidget::on_chkEnabled_stateChanged(int state)
{
    bool enabled = (state == Qt::Checked);
    ui->grpAPIConfiguration->setEnabled(enabled);
}

void LLMSettingsWidget::on_txtEndpoint_textChanged(const QString &text)
{
    // Simple validation: check if endpoint starts with http:// or https://
    if (!text.isEmpty() && !text.startsWith("http://") && !text.startsWith("https://")) {
        ui->lblEndpointWarning->setText(tr("Warning: Endpoint should start with http:// or https://"));
        ui->lblEndpointWarning->setVisible(true);
    } else {
        ui->lblEndpointWarning->setVisible(false);
    }
}
