#include "toolchainsettingswidget.h"
#include "ui_toolchainsettingswidget.h"
#include "directorylistwidget.h"
#include "../settings/toolchainmanager.h"
#include "../settings/buildconfigmanager.h"
#include "../compiler/compilerinfo.h"
#include "../systemconsts.h"
#include "../mainwindow.h"
#include "../utils.h"
#include "../widgets/compileargumentswidget.h"
#include "qt_utils/charsetinfo.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QUuid>

ToolchainSettingsWidget::ToolchainSettingsWidget(const QString& name, const QString& group,
                                                   IconsManager *iconsManager, QWidget *parent)
    : SettingsWidget(name, group, iconsManager, parent)
    , ui(new Ui::ToolchainSettingsWidget)
    , mBinDirWidget(nullptr)
    , mLibDirWidget(nullptr)
    , mCIncludeDirWidget(nullptr)
    , mCppIncludeDirWidget(nullptr)
{
    ui->setupUi(this);
}

ToolchainSettingsWidget::~ToolchainSettingsWidget()
{
    delete ui;
}

void ToolchainSettingsWidget::init()
{
    // Set up directory widgets
    mBinDirWidget = new DirectoryListWidget(iconsManager(), this);
    mLibDirWidget = new DirectoryListWidget(iconsManager(), this);
    mCIncludeDirWidget = new DirectoryListWidget(iconsManager(), this);
    mCppIncludeDirWidget = new DirectoryListWidget(iconsManager(), this);

    ui->dirTabs->addTab(mBinDirWidget, tr("Binaries"));
    ui->dirTabs->addTab(mLibDirWidget, tr("Libraries"));
    ui->dirTabs->addTab(mCIncludeDirWidget, tr("C Includes"));
    ui->dirTabs->addTab(mCppIncludeDirWidget, tr("C++ Includes"));

    SettingsWidget::init();
}

void ToolchainSettingsWidget::doLoad()
{
    ui->cbToolchain->blockSignals(true);
    ui->cbToolchain->clear();

    ToolchainManager& tm = pSettings->toolchainManager();
    if (tm.size() <= 0) {
        ui->btnRename->setEnabled(false);
        ui->btnRemove->setEnabled(false);
        ui->cbToolchain->blockSignals(false);
        return;
    }

    ui->btnRename->setEnabled(true);
    ui->btnRemove->setEnabled(true);

    for (int i = 0; i < tm.size(); i++) {
        ui->cbToolchain->addItem(tm.toolchains()[i].name);
    }

    int index = tm.defaultIndex();
    if (index < 0 || index >= ui->cbToolchain->count())
        index = 0;
    ui->cbToolchain->setCurrentIndex(index);
    ui->cbToolchain->blockSignals(false);
    loadCurrentToolchain();
}

void ToolchainSettingsWidget::doSave()
{
    if (pSettings->toolchainManager().size() > 0) {
        saveCurrentToolchain();
    }
    pSettings->toolchainManager().save(
        pSettings->dirs().config(DirSettings::DataType::None) + "/toolchains.json");
}

QString ToolchainSettingsWidget::currentToolchainId() const
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0 || idx >= pSettings->toolchainManager().size())
        return {};
    return pSettings->toolchainManager().toolchains()[idx].id;
}

void ToolchainSettingsWidget::loadCurrentToolchain()
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0 || idx >= pSettings->toolchainManager().size())
        return;

    Toolchain tc = pSettings->toolchainManager().toolchains()[idx];

    // Programs
    ui->txtCCompiler->setText(tc.ccompiler);
    ui->txtCppCompiler->setText(tc.cppCompiler);
    ui->txtMake->setText(tc.make);
    ui->txtDebugger->setText(tc.debugger);
    ui->txtGDBServer->setText(tc.debugServer);
    ui->txtResourceCompiler->setText(tc.resourceCompiler);

    // Directories
    mBinDirWidget->setDirList(tc.binDirs);
    mLibDirWidget->setDirList(tc.libDirs);
    mCIncludeDirWidget->setDirList(tc.cIncludeDirs);
    mCppIncludeDirWidget->setDirList(tc.cppIncludeDirs);

    // Output suffixes
    ui->txtPreprocessingSuffix->setText(tc.preprocessingSuffix);
    ui->txtCompilationSuffix->setText(tc.compilationProperSuffix);
    ui->txtAssemblingSuffix->setText(tc.assemblingSuffix);
    ui->txtExecutableSuffix->setText(tc.executableSuffix);

    // Misc
    ui->chkPersistInAutoFind->setChecked(false); // Toolchain has no persistInAutoFind field
    ui->chkForceEnglishOutput->setChecked(tc.forceEnglishOutput);
    ui->txtClangTarget->setText(tc.clangTarget);

#ifdef ENABLE_SDCC
    bool isSDCC = (tc.compilerType == CompilerType::SDCC);
    ui->lbBinarySuffix->setVisible(isSDCC);
    ui->cbBinarySuffix->setVisible(isSDCC);
#else
    ui->lbBinarySuffix->setVisible(false);
    ui->cbBinarySuffix->setVisible(false);
#endif

    // Compiler options (toolchain-level)
    ui->optionTabs->resetUI(tc.compilerType, tc.compilerOptions,
                             CompileArgumentsWidget::OptionLevelFilter::Toolchain);
}

void ToolchainSettingsWidget::saveCurrentToolchain()
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0 || idx >= pSettings->toolchainManager().size())
        return;

    Toolchain tc = pSettings->toolchainManager().toolchains()[idx];

    tc.ccompiler = ui->txtCCompiler->text().trimmed();
    tc.cppCompiler = ui->txtCppCompiler->text().trimmed();
    tc.make = ui->txtMake->text().trimmed();
    tc.debugger = ui->txtDebugger->text().trimmed();
    tc.debugServer = ui->txtGDBServer->text().trimmed();
    tc.resourceCompiler = ui->txtResourceCompiler->text().trimmed();

    tc.binDirs = mBinDirWidget->dirList();
    tc.libDirs = mLibDirWidget->dirList();
    tc.cIncludeDirs = mCIncludeDirWidget->dirList();
    tc.cppIncludeDirs = mCppIncludeDirWidget->dirList();

    tc.preprocessingSuffix = ui->txtPreprocessingSuffix->text();
    tc.compilationProperSuffix = ui->txtCompilationSuffix->text();
    tc.assemblingSuffix = ui->txtAssemblingSuffix->text();
    tc.executableSuffix = ui->txtExecutableSuffix->text();

    tc.forceEnglishOutput = ui->chkForceEnglishOutput->isChecked();
    tc.clangTarget = ui->txtClangTarget->text().trimmed();

    tc.compilerOptions = ui->optionTabs->arguments(false);

    pSettings->toolchainManager().updateToolchain(tc.id, tc);
}

// ---- Button handlers ----

void ToolchainSettingsWidget::on_cbToolchain_currentIndexChanged(int /*index*/)
{
    saveCurrentToolchain();
    loadCurrentToolchain();
}

void ToolchainSettingsWidget::on_btnFindCompilers_clicked()
{
#ifdef Q_OS_WIN
    QString msg = tr("Red Panda C++ will clear previously found compiler list and search"
                  " for compilers in the following locations:<br /> '%1'<br /> '%2'<br />Do you really want to continue?")
                             .arg(getFilePath(pSettings->dirs().appDir(), "MinGW32"),
                                  getFilePath(pSettings->dirs().appDir(), "MinGW64"));
#else
    QString msg = tr("Red Panda C++ will clear previously found compiler list and search"
                  " for compilers in the PATH. <br />Do you really want to continue?");
#endif
    if (QMessageBox::warning(this, tr("Confirm"), msg,
                             QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    QProgressDialog progress(tr("Searching for compilers..."), tr("Abort"), 0, 1, pMainWindow);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(2);
    progress.setLabelText(tr("Searching..."));
    pSettings->toolchainManager().discover();
    progress.setValue(1);

    // Create builtin build configs for newly detected compiler types
    BuildConfigManager& bm = pSettings->buildConfigManager();
    QSet<CompilerType> seen;
    for (const Toolchain& tc : pSettings->toolchainManager().toolchains()) {
        if (!seen.contains(tc.compilerType)) {
            seen.insert(tc.compilerType);
            bm.createBuiltinDefaults(tc.compilerType);
        }
    }
    bm.save(pSettings->dirs().config(DirSettings::DataType::None) + "/build_configs.json");

    doLoad();
    progress.setValue(2);
    setSettingsChanged();

    if (pSettings->toolchainManager().size() == 0) {
        QMessageBox::warning(this, tr("Failed"), tr("Can't find any compiler."));
    }
}

void ToolchainSettingsWidget::on_btnAddBlank_clicked()
{
    QString name = QInputDialog::getText(this, tr("Toolchain Name"), tr("Name"));
    name = name.trimmed();
    if (name.isEmpty()) return;

    Toolchain tc;
    tc.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    tc.name = name;
    pSettings->toolchainManager().addToolchain(tc);
    doLoad();
    setSettingsChanged();
}

void ToolchainSettingsWidget::on_btnAddByFolder_clicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Compiler Folder"));
    if (folder.isEmpty()) return;

    int oldSize = pSettings->toolchainManager().size();
    if (!pSettings->toolchainManager().discoverFromFolder(folder)) {
        pSettings->toolchainManager().discoverFromFolder(folder + QDir::separator() + "bin");
    }
    doLoad();
    int newSize = pSettings->toolchainManager().size();
    if (oldSize == newSize) {
        QMessageBox::warning(this, tr("Failed"), tr("Can't find any compiler."));
    } else {
        setSettingsChanged();
    }
}

void ToolchainSettingsWidget::on_btnAddByFile_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Compiler"));
    if (file.isEmpty()) return;
    QFileInfo fileInfo(file);
    if (!fileInfo.isExecutable()) return;

    int oldSize = pSettings->toolchainManager().size();
    pSettings->toolchainManager().discoverFromFolder(fileInfo.absolutePath());
    doLoad();
    int newSize = pSettings->toolchainManager().size();
    if (oldSize == newSize) {
        QMessageBox::warning(this, tr("Failed"), tr("Can't detect compiler from selected file."));
    } else {
        setSettingsChanged();
    }
}

void ToolchainSettingsWidget::on_btnCopy_clicked()
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0 || idx >= pSettings->toolchainManager().size()) return;
    Toolchain source = pSettings->toolchainManager().toolchains()[idx];
    QString name = QInputDialog::getText(this, tr("Toolchain Name"),
        tr("New name"), QLineEdit::Normal, tr("%1 Copy").arg(source.name));
    name = name.trimmed();
    if (name.isEmpty()) return;

    source.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    source.name = name;
    pSettings->toolchainManager().addToolchain(source);
    doLoad();
    setSettingsChanged();
}

void ToolchainSettingsWidget::on_btnRename_clicked()
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0 || idx >= pSettings->toolchainManager().size()) return;
    Toolchain tc = pSettings->toolchainManager().toolchains()[idx];
    QString name = QInputDialog::getText(this, tr("Rename Toolchain"),
        tr("New name"), QLineEdit::Normal, tc.name);
    name = name.trimmed();
    if (name.isEmpty()) return;
    tc.name = name;
    pSettings->toolchainManager().updateToolchain(tc.id, tc);
    doLoad();
    setSettingsChanged();
}

void ToolchainSettingsWidget::on_btnRemove_clicked()
{
    int idx = ui->cbToolchain->currentIndex();
    if (idx < 0 || idx >= pSettings->toolchainManager().size()) return;
    Toolchain tc = pSettings->toolchainManager().toolchains()[idx];
    if (QMessageBox::question(this, tr("Remove"),
            tr("Do you really want to remove \"%1\"?").arg(tc.name),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)
        == QMessageBox::Yes) {
        pSettings->toolchainManager().removeToolchain(tc.id);
        doLoad();
        setSettingsChanged();
    }
}

// ---- File chooser buttons ----

void ToolchainSettingsWidget::on_btnChooseCCompiler_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Choose C Compiler"));
    if (!f.isEmpty()) ui->txtCCompiler->setText(f);
}

void ToolchainSettingsWidget::on_btnChooseCppCompiler_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Choose C++ Compiler"));
    if (!f.isEmpty()) ui->txtCppCompiler->setText(f);
}

void ToolchainSettingsWidget::on_btnChooseMake_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Choose Make"));
    if (!f.isEmpty()) ui->txtMake->setText(f);
}

void ToolchainSettingsWidget::on_btnChooseDebugger_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Choose Debugger"));
    if (!f.isEmpty()) ui->txtDebugger->setText(f);
}

void ToolchainSettingsWidget::on_btnChooseGDBServer_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Choose GDB Server"));
    if (!f.isEmpty()) ui->txtGDBServer->setText(f);
}

void ToolchainSettingsWidget::on_btnChooseResourceCompiler_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Choose Resource Compiler"));
    if (!f.isEmpty()) ui->txtResourceCompiler->setText(f);
}

void ToolchainSettingsWidget::updateIcons(const QSize& /*size*/)
{
}
