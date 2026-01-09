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
#include "llmresultwindow.h"
#include "ui_llmresultwindow.h"
#include <QClipboard>
#include <QApplication>
#include <QScrollBar>

LLMResultWindow::LLMResultWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LLMResultWindow),
    mComplete(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    // Set monospace font for better code readability
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    ui->textResult->setFont(font);
    
    // Initially show loading
    ui->progressBar->setVisible(true);
    ui->btnCopy->setEnabled(false);
}

LLMResultWindow::~LLMResultWindow()
{
    delete ui;
}

void LLMResultWindow::appendText(const QString &text)
{
    QTextCursor cursor = ui->textResult->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text);
    ui->textResult->setTextCursor(cursor);
    
    // Auto-scroll to bottom
    QScrollBar *scrollBar = ui->textResult->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    ui->btnCopy->setEnabled(true);
}

void LLMResultWindow::clearText()
{
    ui->textResult->clear();
    ui->progressBar->setVisible(true);
    ui->btnCopy->setEnabled(false);
    mComplete = false;
}

void LLMResultWindow::showError(const QString &error)
{
    ui->textResult->clear();
    ui->textResult->setPlainText(tr("Error: %1").arg(error));
    ui->progressBar->setVisible(false);
    ui->btnCopy->setEnabled(true);
    mComplete = true;
}

void LLMResultWindow::setComplete(bool complete)
{
    mComplete = complete;
    if (complete) {
        ui->progressBar->setVisible(false);
    }
}

bool LLMResultWindow::isComplete() const
{
    return mComplete;
}

void LLMResultWindow::on_btnCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->textResult->toPlainText());
}

void LLMResultWindow::on_btnClose_clicked()
{
    close();
}
