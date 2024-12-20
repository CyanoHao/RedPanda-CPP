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
#include "compilermanager.h"
#include "filecompiler.h"
#include "../project.h"
#ifdef ENABLE_SDCC
#include "sdccfilecompiler.h"
#include "sdccprojectcompiler.h"
#endif
#include "stdincompiler.h"
#include "../mainwindow.h"
#include "executablerunner.h"
#include "builtinterminalrunner.h"
#include "ojproblemcasesrunner.h"
#include "utils.h"
#include "utils/parsearg.h"
#include "../systemconsts.h"
#include "../settings.h"
#include <QMessageBox>
#include <QUuid>
#include "projectcompiler.h"
#ifdef Q_OS_MACOS
#include <sys/posix_shm.h>
#endif

#ifndef Q_OS_WIN
#include <qtermwidget/qtermwidget.h>
#include <QMenuBar>
#include <QMenu>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

CompilerManager::CompilerManager(QObject *parent) : QObject(parent),
    mCompileMutex(),
    mBackgroundSyntaxCheckMutex(),
    mRunnerMutex()
{
    mCompiler = nullptr;
    mBackgroundSyntaxChecker = nullptr;
    mRunner = nullptr;
    mTerminalWindow = nullptr;
    mSyntaxCheckErrorCount = 0;
    mSyntaxCheckIssueCount = 0;
    mCompileErrorCount = 0;
    mCompileIssueCount = 0;
    mSyntaxCheckErrorCount = 0;
}

CompilerManager::~CompilerManager()
{
    stopAllRunners();
    stopCompile();
    stopCheckSyntax();
    stopRun();
}

bool CompilerManager::compiling()
{
    QMutexLocker locker(&mCompileMutex);
    return mCompiler!=nullptr;
}

bool CompilerManager::backgroundSyntaxChecking()
{
    QMutexLocker locker(&mBackgroundSyntaxCheckMutex);
    return mBackgroundSyntaxChecker!=nullptr;
}

bool CompilerManager::running()
{
    QMutexLocker locker(&mRunnerMutex);
    return (mRunner!=nullptr && !mRunner->pausing());
}

void CompilerManager::compile(const QString& filename, const QByteArray& encoding, bool rebuild, CppCompileType compileType)
{
    if (!pSettings->compilerSets().defaultSet()) {
        QMessageBox::critical(pMainWindow,
                              tr("No compiler set"),
                              tr("No compiler set is configured.")+tr("Can't start debugging."));
        return;
    }
    {
        QMutexLocker locker(&mCompileMutex);
        if (mCompiler!=nullptr) {
            return;
        }
        mCompileErrorCount = 0;
        mCompileIssueCount = 0;
        //deleted when thread finished
#ifdef ENABLE_SDCC
        if (pSettings->compilerSets().defaultSet()->compilerType()==CompilerType::SDCC) {
            mCompiler = new SDCCFileCompiler(filename,encoding,compileType,false);
        } else
#endif
            mCompiler = new FileCompiler(filename,encoding,compileType,false);
        mCompiler->setRebuild(rebuild);
        connect(mCompiler, &Compiler::finished, mCompiler, &QObject::deleteLater);
        connect(mCompiler, &Compiler::compileFinished, this, &CompilerManager::onCompileFinished);
        connect(mCompiler, &Compiler::compileIssue, this, &CompilerManager::onCompileIssue);
        connect(mCompiler, &Compiler::compileStarted, pMainWindow, &MainWindow::onCompileStarted);
        connect(mCompiler, &Compiler::compileStarted, pMainWindow, &MainWindow::clearToolsOutput);

        connect(mCompiler, &Compiler::compileOutput, pMainWindow, &MainWindow::logToolsOutput);
        connect(mCompiler, &Compiler::compileIssue, pMainWindow, &MainWindow::onCompileIssue);
        connect(mCompiler, &Compiler::compileErrorOccured, pMainWindow, &MainWindow::onCompileErrorOccured);
        mCompiler->start();
    }
}

void CompilerManager::compileProject(std::shared_ptr<Project> project, bool rebuild)
{
    if (!pSettings->compilerSets().defaultSet()) {
        QMessageBox::critical(pMainWindow,
                              tr("No compiler set"),
                              tr("No compiler set is configured.")+tr("Can't start debugging."));
        return;
    }
    {
        QMutexLocker locker(&mCompileMutex);
        if (mCompiler!=nullptr) {
            return;
        }
        mCompileErrorCount = 0;
        mCompileIssueCount = 0;
        //deleted when thread finished
        mCompiler = createProjectCompiler(project);
        mCompiler->setRebuild(rebuild);
        connect(mCompiler, &Compiler::finished, mCompiler, &QObject::deleteLater);
        connect(mCompiler, &Compiler::compileFinished, this, &CompilerManager::onCompileFinished);

        connect(mCompiler, &Compiler::compileIssue, this, &CompilerManager::onCompileIssue);
        connect(mCompiler, &Compiler::compileStarted, pMainWindow, &MainWindow::onProjectCompileStarted);
        connect(mCompiler, &Compiler::compileStarted, pMainWindow, &MainWindow::clearToolsOutput);

        connect(mCompiler, &Compiler::compileOutput, pMainWindow, &MainWindow::logToolsOutput);
        connect(mCompiler, &Compiler::compileIssue, pMainWindow, &MainWindow::onCompileIssue);
        connect(mCompiler, &Compiler::compileErrorOccured, pMainWindow, &MainWindow::onCompileErrorOccured);
        mCompiler->start();
    }
}

void CompilerManager::cleanProject(std::shared_ptr<Project> project)
{
    if (!pSettings->compilerSets().defaultSet()) {
        QMessageBox::critical(pMainWindow,
                              tr("No compiler set"),
                              tr("No compiler set is configured.")+tr("Can't start debugging."));
        return;
    }
    {
        QMutexLocker locker(&mCompileMutex);
        if (mCompiler!=nullptr) {
            return;
        }
        mCompileErrorCount = 0;
        mCompileIssueCount = 0;
        //deleted when thread finished
        ProjectCompiler* compiler = createProjectCompiler(project);
        compiler->setOnlyClean(true);
        mCompiler = compiler;
        mCompiler->setRebuild(false);
        connect(mCompiler, &Compiler::finished, mCompiler, &QObject::deleteLater);
        connect(mCompiler, &Compiler::compileFinished, this, &CompilerManager::onCompileFinished);

        connect(mCompiler, &Compiler::compileIssue, this, &CompilerManager::onCompileIssue);
        connect(mCompiler, &Compiler::compileStarted, pMainWindow, &MainWindow::onProjectCompileStarted);
        connect(mCompiler, &Compiler::compileStarted, pMainWindow, &MainWindow::clearToolsOutput);

        connect(mCompiler, &Compiler::compileOutput, pMainWindow, &MainWindow::logToolsOutput);
        connect(mCompiler, &Compiler::compileIssue, pMainWindow, &MainWindow::onCompileIssue);
        connect(mCompiler, &Compiler::compileErrorOccured, pMainWindow, &MainWindow::onCompileErrorOccured);
        mCompiler->start();
    }
}

void CompilerManager::buildProjectMakefile(std::shared_ptr<Project> project)
{
    if (!pSettings->compilerSets().defaultSet()) {
        QMessageBox::critical(pMainWindow,
                              tr("No compiler set"),
                              tr("No compiler set is configured.")+tr("Can't start debugging."));
        return;
    }
    {
        QMutexLocker locker(&mCompileMutex);
        if (mCompiler!=nullptr) {
            return;
        }
        ProjectCompiler* pCompiler=createProjectCompiler(project);
        pCompiler->buildMakeFile();
        delete pCompiler;
    }
}

void CompilerManager::checkSyntax(const QString &filename, const QByteArray& encoding, const QString &content, std::shared_ptr<Project> project)
{
    if (!pSettings->compilerSets().defaultSet()) {
        QMessageBox::critical(pMainWindow,
                              tr("No compiler set"),
                              tr("No compiler set is configured.")+tr("Can't start debugging."));
        return;
    }
    {
        QMutexLocker locker(&mBackgroundSyntaxCheckMutex);
        if (mBackgroundSyntaxChecker!=nullptr) {
            return;
        }

        mSyntaxCheckErrorCount = 0;
        mSyntaxCheckIssueCount = 0;

        //deleted when thread finished
        mBackgroundSyntaxChecker = new StdinCompiler(filename,encoding, content,true);
        mBackgroundSyntaxChecker->setProject(project);
        connect(mBackgroundSyntaxChecker, &Compiler::finished, mBackgroundSyntaxChecker, &QThread::deleteLater);
        connect(mBackgroundSyntaxChecker, &Compiler::compileIssue, this, &CompilerManager::onSyntaxCheckIssue);
        connect(mBackgroundSyntaxChecker, &Compiler::compileStarted, pMainWindow, &MainWindow::onSyntaxCheckStarted);
        connect(mBackgroundSyntaxChecker, &Compiler::compileFinished, this, &CompilerManager::onSyntaxCheckFinished);
        //connect(mBackgroundSyntaxChecker, &Compiler::compileOutput, pMainWindow, &MainWindow::logToolsOutput);
        connect(mBackgroundSyntaxChecker, &Compiler::compileIssue, pMainWindow, &MainWindow::onCompileIssue);
        connect(mBackgroundSyntaxChecker, &Compiler::compileErrorOccured, pMainWindow, &MainWindow::onCompileErrorOccured);
        mBackgroundSyntaxChecker->start();
    }
}

CompilerManager::ExecArgs CompilerManager::resolveExecArgs(const QString &filename, const QString &arguments)
{
    using TerminalMode = Settings::Environment::TerminalMode;

    QStringList execArgs;
    QString sharedMemoryId;
    QString redirectInputFilename;

    bool redirectInput = pSettings->executor().redirectInput();
    if (redirectInput)
        redirectInputFilename = pSettings->executor().inputFilename();
    bool hasDiscreteTty = pSettings->environment().terminalMode() != TerminalMode(TerminalMode::WindowsDefault);
    ExecutableRunner * execRunner;
    if (programIsWin32GuiApp(filename)) {
        QString consolePauserPath = getFilePath(pSettings->dirs().appLibexecDir(), CONSOLE_PAUSER);
        QStringList execArgs = {consolePauserPath};
        if (!redirectInputFilename.isEmpty()) {
            if (hasDiscreteTty)
                execArgs << "--redirect-input" << redirectInputFilename;
            else
                execArgs << "--redirect-input" << "-";
        }
        if (pSettings->executor().pauseConsole())
            execArgs << "--pause-console";
#ifdef Q_OS_WIN
    if (pSettings->executor().enableVirualTerminalSequence())
        execArgs << "--enable-virtual-terminal-sequence";
    QString triplet = pSettings->compilerSets().defaultSet()->dumpMachine();
    if (triplet.contains("-linux-"))
        execArgs << "--run-in-wsl";
    sharedMemoryId = QUuid::createUuid().toString();
#else
    sharedMemoryId = "/r" + QUuid::createUuid().toString(QUuid::StringFormat::Id128);
#ifdef Q_OS_MACOS
    sharedMemoryId = sharedMemoryId.mid(0, PSHMNAMLEN);
#endif
#endif
    bool requireConsolePauser = execArgs.length() > 1;
    if (requireConsolePauser) {
        if (!fileExists(consolePauserPath)) {
            QMessageBox::critical(pMainWindow,
                                        tr("Can't find Console Pauser"),
                                        tr("Console Pauser \"%1\" doesn't exists!")
                                        .arg(consolePauserPath));
            return {};
        }
        execArgs << "--shared-memory" << sharedMemoryId;

        // translation items: please keep sync with `tools/consolepauser/argparser.hpp`
        execArgs << "--add-translation" << "EXIT=" + tr("Press ANY key to exit...");
        execArgs << "--add-translation" << "USAGE_HEADER=" + tr("Process exited after");
        execArgs << "--add-translation" << "USAGE_RETURN_VALUE=" + tr("Return value");
        execArgs << "--add-translation" << "USAGE_CPU_TIME=" + tr("CPU time");
        execArgs << "--add-translation" << "USAGE_MEMORY=" + tr("Memory");

        // end of console pauser args
        execArgs << "--";
    } else {
        execArgs.clear();
        sharedMemoryId.clear();
    }
    execArgs << localizePath(filename);
    execArgs += parseArgumentsWithoutVariables(arguments);
    return ExecArgs{execArgs, sharedMemoryId, redirectInputFilename};
}

void CompilerManager::runInExternalTerminal(
        const QString &filename,
        const QString &arguments,
        const QString &workDir,
        const QStringList& binDirs)
{
    using TerminalMode = Settings::Environment::TerminalMode;
    QMutexLocker locker(&mRunnerMutex);
    if (mRunner!=nullptr && !mRunner->pausing()) {
        return;
    }
    ExecutableRunner * execRunner;

    auto [
        execArgs,
        sharedMemoryId,
        redirectInputFilename
    ] = resolveExecArgs(filename, arguments);

    if (execArgs.empty()) {
        // error: console pauser not found
        return;
    }

    bool useCustomTerminal = pSettings->environment().terminalMode() == TerminalMode(TerminalMode::External);
    bool redirectInput = !redirectInputFilename.isEmpty();

    if (useCustomTerminal) {
        auto [filename, args, fileOwner] = wrapCommandForTerminalEmulator(
            pSettings->environment().terminalPath(),
            pSettings->environment().terminalArgumentsPattern(),
            execArgs
        );
        //delete when thread finished
        execRunner = new ExecutableRunner(filename, args, workDir);
        mTempFileOwner = std::move(fileOwner);
    } else {
        //delete when thread finished
        execRunner = new ExecutableRunner(execArgs[0], execArgs.mid(1), workDir);
    }
    execRunner->setShareMemoryId(sharedMemoryId);
    execRunner->setStartConsole(true);

    // redirection: custom terminal has discrete tty, console pauser will handle redirection
    if (redirectInput && !useCustomTerminal) {
        execRunner->setRedirectInput(true);
        execRunner->setRedirectInputFilename(redirectInputFilename);
    }

    execRunner->addBinDirs(binDirs);
    execRunner->addBinDir(pSettings->dirs().appDir());

    mRunner = execRunner;
    connect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
    connect(mRunner, &Runner::finished, mRunner ,&Runner::deleteLater);
    connect(mRunner, &Runner::finished, pMainWindow ,&MainWindow::onRunFinished);
    connect(mRunner, &Runner::pausingForFinish, pMainWindow ,&MainWindow::onRunPausingForFinish);
    connect(mRunner, &Runner::pausingForFinish, this ,&CompilerManager::onRunnerPausing);
    connect(mRunner, &Runner::runErrorOccurred, pMainWindow ,&MainWindow::onRunErrorOccured);
    mRunner->start();
}

#ifndef Q_OS_WIN
void CompilerManager::runInBuiltInTerminal(
        const QString &filename,
        const QString &arguments,
        const QString &workDir,
        const QStringList& binDirs)
{
    struct AutoDestroyMainWindow : public QMainWindow
    {
        void closeEvent(QCloseEvent *event) override
        {
            deleteLater();
            event->accept();
        }
    };

    QMainWindow *mainWindow = new AutoDestroyMainWindow();
    QTermWidget *console = new QTermWidget(0, mainWindow);

    // set up terminal window
    QMenuBar *menuBar = new QMenuBar(mainWindow);
    QMenu *actionsMenu = new QMenu(QStringLiteral("Actions"), menuBar);
    menuBar->addMenu(actionsMenu);
    actionsMenu->addAction(QStringLiteral("Find..."), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), 
        console, &QTermWidget::toggleShowSearchBar);
    actionsMenu->addAction(QStringLiteral("Copy"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), 
        console, &QTermWidget::copyClipboard);
    actionsMenu->addAction(QStringLiteral("Paste"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), 
        console, &QTermWidget::pasteClipboard);
    mainWindow->setMenuBar(menuBar);
    QFont font("Monospace", 12);
    console->setTerminalFont(font);
    console->setScrollBarPosition(QTermWidget::ScrollBarRight);
    mainWindow->setCentralWidget(console);
    mainWindow->resize(600, 400);

    auto [
        execArgs,
        sharedMemoryId,
        redirectInputFilename
    ] = resolveExecArgs(filename, arguments);

    if (execArgs.empty()) {
        // error: console pauser not found
        return;
    }

    console->setShellProgram(execArgs[0]);
    console->setArgs(execArgs.mid(1));
    console->setWorkingDirectory(workDir);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value("PATH");
    QStringList pathAdded = binDirs;
    if (!path.isEmpty()) {
        path = pathAdded.join(PATH_SEPARATOR) + PATH_SEPARATOR + path;
    } else {
        path = pathAdded.join(PATH_SEPARATOR);
    }
    env.insert("PATH",path);
    console->setEnvironment(env.toStringList());

    constexpr size_t BUF_SIZE = BuiltInTerminalRunner::sharedMemorySize;
    char* pBuf=nullptr;
    int fd_shm = shm_open(sharedMemoryId.toLocal8Bit().data(),O_RDWR | O_CREAT,S_IRWXU);
    if (fd_shm==-1) {
        qDebug()<<QString("shm open failed %1:%2").arg(errno).arg(strerror(errno));
    } else {
        if (ftruncate(fd_shm, BUF_SIZE)==-1){
            qDebug()<<QString("truncate failed %1:%2").arg(errno).arg(strerror(errno));
        } else {
            pBuf = (char*)mmap(NULL,BUF_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm,0);
            if (pBuf == MAP_FAILED) {
                qDebug()<<QString("mmap failed %1:%2").arg(errno).arg(strerror(errno));
                pBuf = nullptr;
            }
        }
    }

    mTerminalWindow = mainWindow;
    mRunner = new BuiltInTerminalRunner(fd_shm, pBuf, sharedMemoryId);

    connect(console, &QTermWidget::finished, mainWindow, &QMainWindow::close);
    connect(console, &QTermWidget::finished, mRunner, &Runner::stop);

    connect(mTerminalWindow, &QMainWindow::destroyed, mRunner, &Runner::stop);

    connect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
    connect(mRunner, &Runner::finished, mRunner ,&Runner::deleteLater);
    connect(mRunner, &Runner::finished, pMainWindow ,&MainWindow::onRunFinished);
    connect(mRunner, &Runner::pausingForFinish, pMainWindow ,&MainWindow::onRunPausingForFinish);
    connect(mRunner, &Runner::pausingForFinish, this ,&CompilerManager::onRunnerPausing);
    connect(mRunner, &Runner::runErrorOccurred, pMainWindow ,&MainWindow::onRunErrorOccured);
    mRunner->start();

    mainWindow->show();
    console->startShellProgram();
}
#endif

void CompilerManager::runWithoutTerminal(const QString &filename, const QString &arguments, const QString &workDir, const QStringList& binDirs)
{
    ExecutableRunner *execRunner = new ExecutableRunner(filename, parseArgumentsWithoutVariables(arguments), workDir);
    execRunner->setStartConsole(false);
    execRunner->addBinDirs(binDirs);
    execRunner->addBinDir(pSettings->dirs().appDir());

    mRunner = execRunner;
    connect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
    connect(mRunner, &Runner::finished, mRunner ,&Runner::deleteLater);
    connect(mRunner, &Runner::finished, pMainWindow ,&MainWindow::onRunFinished);
    connect(mRunner, &Runner::pausingForFinish, pMainWindow ,&MainWindow::onRunPausingForFinish);
    connect(mRunner, &Runner::pausingForFinish, this ,&CompilerManager::onRunnerPausing);
    connect(mRunner, &Runner::runErrorOccurred, pMainWindow ,&MainWindow::onRunErrorOccured);
    mRunner->start();
}

void CompilerManager::run(const QString &filename, const QString &arguments, const QString &workDir, const QStringList& binDirs)
{
    using TerminalMode = Settings::Environment::TerminalMode;

    if (!programHasConsole(filename))
        return runWithoutTerminal(filename, arguments, workDir, binDirs);

    switch (pSettings->environment().terminalMode()) {
#ifndef Q_OS_WIN
        case TerminalMode::BuiltInPanel:
        case TerminalMode::BuiltInWindow:
            runInBuiltInTerminal(filename, arguments, workDir, binDirs);
            break;
#endif
        case TerminalMode::External:
        case TerminalMode::WindowsDefault:
            runInExternalTerminal(filename, arguments, workDir, binDirs);
            break;
    }
}

void CompilerManager::runProblem(const QString &filename, const QString &arguments, const QString &workDir, POJProblemCase problemCase,
                                 const POJProblem& problem
                                 )
{
    QMutexLocker locker(&mRunnerMutex);
    doRunProblem(filename, arguments, workDir, QVector<POJProblemCase> {problemCase}, problem);

}

void CompilerManager::runProblem(const QString &filename, const QString &arguments, const QString &workDir, const QVector<POJProblemCase>& problemCases,
                                 const POJProblem& problem
                                 )
{
    QMutexLocker locker(&mRunnerMutex);
    doRunProblem(filename, arguments, workDir, problemCases, problem);
}
void CompilerManager::doRunProblem(const QString &filename, const QString &arguments, const QString &workDir, const QVector<POJProblemCase>& problemCases,
                                   const POJProblem& problem)
{
    if (mRunner!=nullptr) {
        return;
    }
    OJProblemCasesRunner * execRunner = new OJProblemCasesRunner(filename, parseArgumentsWithoutVariables(arguments), workDir, problemCases);
    mRunner = execRunner;
    if (pSettings->executor().enableCaseLimit()) {
        execRunner->setExecTimeout(pSettings->executor().caseTimeout());
        execRunner->setMemoryLimit(pSettings->executor().caseMemoryLimit()*1024); //convert kb to bytes
    }
    size_t timeLimit = problem->getTimeLimit();
    size_t memoryLimit = problem->getMemoryLimit();
    if (timeLimit>0)
        execRunner->setExecTimeout(timeLimit);
    if (memoryLimit)
        execRunner->setMemoryLimit(memoryLimit);
    connect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
    connect(mRunner, &Runner::finished, mRunner ,&Runner::deleteLater);
    connect(mRunner, &Runner::finished, pMainWindow ,&MainWindow::onRunProblemFinished);
    connect(mRunner, &Runner::runErrorOccurred, pMainWindow ,&MainWindow::onRunErrorOccured);
    connect(execRunner, &OJProblemCasesRunner::caseStarted, pMainWindow, &MainWindow::onOJProblemCaseStarted);
    connect(execRunner, &OJProblemCasesRunner::caseFinished, pMainWindow, &MainWindow::onOJProblemCaseFinished);
    connect(execRunner, &OJProblemCasesRunner::newOutputGetted, pMainWindow, &MainWindow::onOJProblemCaseNewOutputGetted);
    connect(execRunner, &OJProblemCasesRunner::resetOutput, pMainWindow, &MainWindow::onOJProblemCaseResetOutput);
    if (pSettings->executor().redirectStderrToToolLog()) {
        connect(execRunner, &OJProblemCasesRunner::logStderrOutput, pMainWindow, &MainWindow::logToolsOutput);
    }
    mRunner->start();
}

void CompilerManager::stopRun()
{
    QMutexLocker locker(&mRunnerMutex);
    if (mRunner!=nullptr) {
        mRunner->stop();
        disconnect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
        mRunner=nullptr;
        mTempFileOwner=nullptr;
    }
    if (mTerminalWindow != nullptr) {
        mTerminalWindow->close();
        mTerminalWindow = nullptr;
    }
}

void CompilerManager::stopAllRunners()
{
    emit signalStopAllRunners();
}

void CompilerManager::stopPausing()
{
    QMutexLocker locker(&mRunnerMutex);
    if (mRunner!=nullptr && mRunner->pausing()) {
        disconnect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
        mRunner->stop();
        mRunner=nullptr;
        mTempFileOwner=nullptr;
    }
    if (mTerminalWindow != nullptr) {
        mTerminalWindow->close();
        mTerminalWindow = nullptr;
    }
}

void CompilerManager::stopCompile()
{
    QMutexLocker locker(&mCompileMutex);
    if (mCompiler!=nullptr)
        mCompiler->stopCompile();
}

void CompilerManager::stopCheckSyntax()
{
    QMutexLocker locker(&mBackgroundSyntaxCheckMutex);
    if (mBackgroundSyntaxChecker!=nullptr)
        mBackgroundSyntaxChecker->stopCompile();
}

bool CompilerManager::canCompile(const QString &)
{
    return !compiling();
}

void CompilerManager::onCompileFinished(const QString& filename)
{
    QMutexLocker locker(&mCompileMutex);
    mCompiler=nullptr;
    emit compileFinished(filename, false);
    //pMainWindow->onCompileFinished(filename,false);
}

void CompilerManager::onRunnerTerminated()
{
    QMutexLocker locker(&mRunnerMutex);
    mRunner=nullptr;
    mTempFileOwner=nullptr;
    mTerminalWindow=nullptr;
}

void CompilerManager::onRunnerPausing()
{
    QMutexLocker locker(&mRunnerMutex);
    disconnect(mRunner, &Runner::finished, this ,&CompilerManager::onRunnerTerminated);
    disconnect(mRunner, &Runner::finished, pMainWindow ,&MainWindow::onRunFinished);
    disconnect(mRunner, &Runner::runErrorOccurred, pMainWindow ,&MainWindow::onRunErrorOccured);
    connect(this, &CompilerManager::signalStopAllRunners, mRunner, &Runner::stop);
    mRunner=nullptr;
    mTempFileOwner=nullptr;
    mTerminalWindow = nullptr;
}

void CompilerManager::onCompileIssue(PCompileIssue issue)
{
    if (issue->type == CompileIssueType::Error)
        mCompileErrorCount++;
    mCompileIssueCount++;
}

void CompilerManager::onSyntaxCheckFinished(const QString& filename)
{
    QMutexLocker locker(&mBackgroundSyntaxCheckMutex);
    mBackgroundSyntaxChecker=nullptr;
    emit compileFinished(filename, true);
}

void CompilerManager::onSyntaxCheckIssue(PCompileIssue issue)
{
    if (issue->type == CompileIssueType::Error)
        mSyntaxCheckErrorCount++;
    if (issue->type == CompileIssueType::Error ||
            issue->type == CompileIssueType::Warning)
        mSyntaxCheckIssueCount++;
}

ProjectCompiler *CompilerManager::createProjectCompiler(std::shared_ptr<Project> project)
{
#ifdef ENABLE_SDCC
    if (project->options().type==ProjectType::MicroController)
        return new SDCCProjectCompiler(project);
    else
#endif
        return new ProjectCompiler(project);
}

int CompilerManager::syntaxCheckIssueCount() const
{
    return mSyntaxCheckIssueCount;
}

int CompilerManager::compileIssueCount() const
{
    return mCompileIssueCount;
}

int CompilerManager::syntaxCheckErrorCount() const
{
    return mSyntaxCheckErrorCount;
}

int CompilerManager::compileErrorCount() const
{
    return mCompileErrorCount;
}

CompileError::CompileError(const QString &reason):BaseError(reason)
{

}

