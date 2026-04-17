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
#include "builtinterminalrunner.h"
#include "qtermwidget.h"
#include <QDebug>
#include <QCoreApplication>

BuiltInTerminalRunner::BuiltInTerminalRunner(QTermWidget* terminal, const QString& program,
                                             const QStringList& args, const QString& workDir,
                                             QObject* parent)
    : QObject(parent), mTerminal(terminal), mProgram(program), mArguments(args),
      mWorkDir(workDir), mStarted(false), mFinished(false)
{
    // Connect process signals
    connect(&mProcess, &QProcess::started, this, &BuiltInTerminalRunner::onProcessStarted);
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BuiltInTerminalRunner::onProcessFinished);
    connect(&mProcess, &QProcess::errorOccurred,
            this, &BuiltInTerminalRunner::onProcessError);
    // Connect terminal output
    connect(mTerminal, &QTermWidget::receivedData,
            this, &BuiltInTerminalRunner::onTerminalOutput);
}

BuiltInTerminalRunner::~BuiltInTerminalRunner()
{
    if (isRunning()) {
        stop();
        wait(5000);
    }
}

void BuiltInTerminalRunner::start()
{
    if (mStarted || mFinished) return;
    mStarted = true;
    mFinished = false;
    setupTimer();
    // Start the process
    mProcess.setProgram(mProgram);
    mProcess.setArguments(mArguments);
    mProcess.setWorkingDirectory(mWorkDir);
    mProcess.start();
}

void BuiltInTerminalRunner::stop()
{
    mFinished = true;
    if (mProcess.state() == QProcess::Running) {
        mProcess.terminate();
        mProcess.waitForFinished(5000);
    }
}

bool BuiltInTerminalRunner::isRunning() const
{
    return mStarted && !mFinished && mProcess.state() == QProcess::Running;
}

void BuiltInTerminalRunner::run()
{
    // Run in a separate thread context
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void BuiltInTerminalRunner::onProcessStarted()
{
    qDebug() << "Process started:" << mProgram;
}

void BuiltInTerminalRunner::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    qDebug() << "Process finished:" << exitCode << status;
    mFinished = true;
    emit finished(exitCode);
}

void BuiltInTerminalRunner::onProcessError(QProcess::ProcessError error)
{
    qDebug() << "Process error:" << error;
    mFinished = true;
    switch (error) {
    case QProcess::FailedToStart:
        emit errorOccurred(tr("Failed to start process"));
        break;
    case QProcess::Crashed:
        emit errorOccurred(tr("Process crashed"));
        break;
    case QProcess::Timedout:
        emit errorOccurred(tr("Process timed out"));
        break;
    case QProcess::WriteError:
        emit errorOccurred(tr("Write error"));
        break;
    case QProcess::ReadError:
        emit errorOccurred(tr("Read error"));
        break;
    default:
        emit errorOccurred(tr("Unknown error"));
    }
}

void BuiltInTerminalRunner::onTerminalOutput(const QString& newtext)
{
    emit outputReceived(newtext);
}

void BuiltInTerminalRunner::onTimeout()
{
    // Timeout handling if needed
}

void BuiltInTerminalRunner::setupTimer()
{
    mTimeoutTimer.setSingleShot(true);
    connect(&mTimeoutTimer, &QTimer::timeout, this, &BuiltInTerminalRunner::onTimeout);
}
