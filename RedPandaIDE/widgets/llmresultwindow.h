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
#ifndef LLMRESULTWINDOW_H
#define LLMRESULTWINDOW_H

#include <QDialog>

namespace Ui {
class LLMResultWindow;
}

class LLMResultWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LLMResultWindow(QWidget *parent = nullptr);
    ~LLMResultWindow();

    void appendText(const QString &text);
    void clearText();
    void showError(const QString &error);
    void setComplete(bool complete);
    bool isComplete() const;

private slots:
    void on_btnCopy_clicked();
    void on_btnClose_clicked();

private:
    Ui::LLMResultWindow *ui;
    bool mComplete;
};

#endif // LLMRESULTWINDOW_H
