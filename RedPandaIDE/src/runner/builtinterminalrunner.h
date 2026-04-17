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
#ifndef BUILTINTERMINALRUNNER_H
#define BUILTINTERMINALRUNNER_H

#include "../irunner.h"
#include <QThread>
#include <QObject>
#include <QProcess>
#include <QTimer>

class QTermWidget;

class BuiltInTerminalRunner : public QObject, public IRunner {
    Q_OBJECT
public:
    explicit BuiltInTerminalRunner(QTermWidget* terminal, const QString& program, 
                                   const QStringList& args, const QString& workDir,
                                   QObject* parent = nullptr);
    ~BuiltInTerminalRunner() override;

    // IRunner interface
    void start() override;
    void stop() override;
    bool isRunning() const override;

signals:
    void outputReceived(const QString& output);
    void finished(int exitCode);
    void errorOccurred(const QString& error);

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);
    void onTerminalOutput(const QString& newtext);
    void onTimeout();

private:
    void run() override;
    bool sendCommandToTerminal();
    void setupTimer();

    QTermWidget* mTerminal;
    QString mProgram;
    QStringList mArguments;
    QString mWorkDir;
    QProcess mProcess;
    QTimer mTimeoutTimer;
    bool mStarted;
    bool mFinished;
};

#endif // BUILTINTERMINALRUNNER_H
