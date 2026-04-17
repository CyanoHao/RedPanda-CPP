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
#ifndef EXECUTABLERUNNER_H
#define EXECUTABLERUNNER_H

#include "runner.h"
#include "../irunner.h"
#include <QProcess>
#include <QSemaphore>
#include <memory>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class ExecutableRunner : public Runner, public IRunner {
    Q_OBJECT
public:
    ExecutableRunner(const QString& filename, const QStringList& arguments, const QString& workDir,
                     QObject* parent = nullptr);
    ExecutableRunner(const ExecutableRunner&)=delete;
    ExecutableRunner& operator=(const ExecutableRunner&)=delete;

    const QString &redirectInputFilename() const;
    void setRedirectInputFilename(const QString &newDataFile);

    bool redirectInput() const;
    void setRedirectInput(bool isRedirect);

    bool startConsole() const;
    void setStartConsole(bool newStartConsole);

    const QString &shareMemoryId() const;
    void setShareMemoryId(const QString &newShareMemoryId);

    const QStringList &binDirs() const;
    void addBinDirs(const QStringList &binDirs);
    void addBinDir(const QString &binDir);

    // WSL support
    void setWSLDistro(const QString& distro);
    bool isWSLMode() const;
    QString wslDistro() const;

    // Terminal mode support
    void setTerminalMode(int mode);
    int terminalMode() const;

    // IRunner interface
    void start() override;
    void stop() override;
    bool isRunning() const override;

private:
    QString mRedirectInputFilename;
    QString mShareMemoryId;
    bool mRedirectInput;
    bool mStartConsole;
    QSemaphore mQuitSemaphore;
    QStringList mBinDirs;
    QString mWSLDistro;
    int mTerminalMode;
    std::shared_ptr<QProcess> mProcess;
#ifdef Q_OS_WIN
    HANDLE mSharedMemoryHandle;
#endif

    // QThread interface
protected:
    void run() override;

    // Runner interface
protected:
    void doStop() override;
};

#endif // EXECUTABLERUNNER_H
