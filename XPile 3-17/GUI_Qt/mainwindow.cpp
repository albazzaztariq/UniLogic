// mainwindow.cpp -- Main application window implementation

#include "mainwindow.h"
#include "newprojecttab.h"
#include "drconfiguratortab.h"
#include "buildtab.h"
#include "drconflicttab.h"
#include "packagestab.h"
#include "editortab.h"
#include "repltab.h"
#include "filetreepanel.h"
#include "welcomescreen.h"
#include "outputpanel.h"
#include "settingsdialog.h"
#include "drdiffviewer.h"
#include "normdiffviewer.h"
#include "projectstats.h"
#include "commandpalette.h"
#include "exportdialog.h"
#include "helpdialogs.h"
#include "pluginmanager.h"
#include "updatechecker.h"
#include "setupwizard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QTimer>
#include <QFileDialog>
#include <QShortcut>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

static const char *APP_VERSION = "0.1.0";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("UniLogic Project Manager");
    setMinimumSize(700, 500);

    QString iconPath = QDir(QApplication::applicationDirPath()).filePath("unilogic.ico");
    if (!QFile::exists(iconPath))
        iconPath = QDir(QApplication::applicationDirPath()).filePath("../GUI/unilogic.ico");
    if (QFile::exists(iconPath))
        setWindowIcon(QIcon(iconPath));

    // ── Menu bar ────────────────────────────────────────────────────────
    auto *mb = new QMenuBar(this);

    m_fileMenu = mb->addMenu("&File");
    m_fileMenu->addAction("&New Project",      this, &MainWindow::onNewProject);
    m_fileMenu->addAction("&Open Project...",  this, &MainWindow::onOpenProject);
    m_fileMenu->addSeparator();
    m_recentMenu = m_fileMenu->addMenu("Recent Projects");
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("New &Window",       this, &MainWindow::newWindow);
    m_fileMenu->addAction("&Export Project...", this, &MainWindow::exportProject);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("&Settings...",
                           QKeySequence(Qt::CTRL | Qt::Key_Comma),
                           this, &MainWindow::openSettings);

    m_toolsMenu = mb->addMenu("&Tools");
    m_toolsMenu->addAction("Show &Normalization...", this, &MainWindow::showNormDiff);

    m_analysisMenu = mb->addMenu("&Analysis");
    m_analysisMenu->addAction("DR &Diff...",           this, &MainWindow::showDRDiff);
    m_analysisMenu->addAction("Project &Statistics...", this, &MainWindow::showProjectStats);

    m_helpMenu = mb->addMenu("&Help");
    m_helpMenu->addAction("&Documentation",         this, &MainWindow::openDocumentation);
    m_helpMenu->addAction("&Keyboard Shortcuts...", this, &MainWindow::showShortcuts);
    m_helpMenu->addAction("&Plugins...",            this, &MainWindow::showPlugins);
    m_helpMenu->addAction("Check for &Updates",     this, &MainWindow::checkForUpdates);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction("&About...",              this, &MainWindow::showAbout);

    setMenuBar(mb);

    // ── Status bar (Task 9) ─────────────────────────────────────────────
    statusBar()->showMessage("Ready");

    // ── Central widget ──────────────────────────────────────────────────
    auto *central = new QWidget;
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Header bar ──────────────────────────────────────────────────────
    m_headerBar = new QFrame;
    m_headerBar->setFixedHeight(48);
    auto *hbLayout = new QHBoxLayout(m_headerBar);
    hbLayout->setContentsMargins(20, 0, 20, 0);

    m_titleLbl = new QLabel(QStringLiteral(
        "<span style=\"font-size:28px; vertical-align:middle;\">\u221e</span>"
        "<span style=\"font-size:20px; vertical-align:middle;\"> UniLogic Project Manager</span>"));
    m_titleLbl->setStyleSheet("color: white; font-weight: bold; background: transparent;");
    m_titleLbl->setAlignment(Qt::AlignVCenter);
    hbLayout->addWidget(m_titleLbl);
    hbLayout->addStretch();

    m_versionLbl = new QLabel(QStringLiteral("XPile 3-17  v%1").arg(APP_VERSION));
    m_versionLbl->setToolTip("Application version");
    hbLayout->addWidget(m_versionLbl);

    m_settingsBtn = new QPushButton(QStringLiteral("\u2699"));
    m_settingsBtn->setFixedSize(36, 36);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setToolTip("Open application settings (Ctrl+,)");
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::openSettings);
    hbLayout->addWidget(m_settingsBtn);

    m_themeBtn = new QPushButton;
    m_themeBtn->setFixedSize(36, 36);
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setToolTip("Toggle light/dark theme");
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    hbLayout->addWidget(m_themeBtn);

    mainLayout->addWidget(m_headerBar);

    // ── Update notification bar (hidden by default) ─────────────────────
    m_updateBar = new QFrame;
    m_updateBar->setFixedHeight(32);
    m_updateBar->setVisible(false);
    auto *ubLayout = new QHBoxLayout(m_updateBar);
    ubLayout->setContentsMargins(20, 0, 20, 0);
    m_updateLbl = new QLabel;
    m_updateLbl->setStyleSheet("background: transparent; color: white; font-weight: bold;");
    ubLayout->addWidget(m_updateLbl);
    ubLayout->addStretch();
    mainLayout->addWidget(m_updateBar);

    // ── Colors ──────────────────────────────────────────────────────────
    QString themeName = SettingsDialog::loadEditorTheme();
    m_colors = colorsForThemeName(themeName);

    // ── Stacked widget ──────────────────────────────────────────────────
    m_stack = new QStackedWidget;

    m_welcome = new WelcomeScreen(m_colors);
    connect(m_welcome, &WelcomeScreen::newProjectClicked,    this, &MainWindow::onNewProject);
    connect(m_welcome, &WelcomeScreen::openProjectClicked,   this, &MainWindow::onOpenProject);
    connect(m_welcome, &WelcomeScreen::quickSetupClicked,    this, &MainWindow::runQuickSetup);
    connect(m_welcome, &WelcomeScreen::recentProjectClicked, this, &MainWindow::openProject);
    m_stack->addWidget(m_welcome);

    // Workspace
    auto *wsWidget = new QWidget;
    auto *wsLayout = new QVBoxLayout(wsWidget);
    wsLayout->setContentsMargins(0, 0, 0, 0);
    wsLayout->setSpacing(0);

    m_outerSplit = new QSplitter(Qt::Vertical);
    m_splitter   = new QSplitter(Qt::Horizontal);

    m_fileTree = new FileTreePanel(m_colors);
    m_splitter->addWidget(m_fileTree);

    m_newProjectTab = new NewProjectTab(m_colors);
    m_drConfigTab   = new DRConfiguratorTab(m_colors);
    m_buildTab      = new BuildTab(m_colors);
    m_drConflictTab = new DRConflictTab(m_colors);
    m_packagesTab   = new PackagesTab(m_colors);
    m_editorTab     = new EditorTab(m_colors);
    m_replTab       = new ReplTab(m_colors);

    m_tabs = new QTabWidget;
    m_tabs->addTab(m_editorTab,     "  Editor  ");
    m_tabs->addTab(m_newProjectTab, "  New Project  ");
    m_tabs->addTab(m_drConfigTab,   "  DR Config  ");
    m_tabs->addTab(m_buildTab,      "  Build  ");
    m_tabs->addTab(m_drConflictTab, "  DR Conflicts  ");
    m_tabs->addTab(m_packagesTab,   "  Packages  ");
    m_tabs->addTab(m_replTab,       "  REPL  ");

    m_splitter->addWidget(m_tabs);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({220, 780});

    m_outerSplit->addWidget(m_splitter);

    m_outputPanel = new OutputPanel(m_colors);
    m_outerSplit->addWidget(m_outputPanel);
    m_outerSplit->setStretchFactor(0, 1);
    m_outerSplit->setStretchFactor(1, 0);
    m_outerSplit->setSizes({500, 220});

    wsLayout->addWidget(m_outerSplit);
    m_stack->addWidget(wsWidget);
    mainLayout->addWidget(m_stack);

    // ── Connect signals ─────────────────────────────────────────────────
    connect(this, &MainWindow::themeChanged, m_newProjectTab, &NewProjectTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_drConfigTab,   &DRConfiguratorTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_buildTab,      &BuildTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_drConflictTab, &DRConflictTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_packagesTab,   &PackagesTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_editorTab,     &EditorTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_replTab,       &ReplTab::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_fileTree,      &FileTreePanel::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_welcome,       &WelcomeScreen::onThemeChanged);
    connect(this, &MainWindow::themeChanged, m_outputPanel,   &OutputPanel::onThemeChanged);

    connect(m_fileTree, &FileTreePanel::fileActivated, this, &MainWindow::openFileInEditor);
    connect(m_newProjectTab, &NewProjectTab::projectCreated, this, &MainWindow::openProject);
    connect(m_outputPanel, &OutputPanel::problemClicked, this, &MainWindow::goToFileAndLine);

    connect(m_buildTab, &BuildTab::buildLogLine,  m_outputPanel, &OutputPanel::appendBuildLine);
    connect(m_buildTab, &BuildTab::runLogLine,    m_outputPanel, &OutputPanel::appendRunLine);
    connect(m_buildTab, &BuildTab::buildStarted,  m_outputPanel, &OutputPanel::clearBuild);
    connect(m_buildTab, &BuildTab::runStarted,    m_outputPanel, &OutputPanel::clearRun);
    connect(m_buildTab, &BuildTab::buildProblem, this,
            [this](const QString &f, int l, int c, const QString &sev, const QString &msg) {
                m_outputPanel->addProblem(f, l, c, sev, msg);
            });
    connect(m_buildTab, &BuildTab::buildStarted, m_outputPanel, &OutputPanel::clearProblems);
    connect(m_buildTab, &BuildTab::buildStarted, this, [this]() { setStatusMessage("Building..."); });
    connect(m_buildTab, &BuildTab::runStarted,   this, [this]() { setStatusMessage("Running..."); });

    setupShortcuts();
    applyTheme();
    updateRecentMenu();

    // ── Plugin manager ──────────────────────────────────────────────────
    m_pluginMgr = new PluginManager(this, this);
    m_pluginMgr->loadPlugins();

    // ── Update checker (background) ─────────────────────────────────────
    m_updateChecker = new UpdateChecker(APP_VERSION, this);
    connect(m_updateChecker, &UpdateChecker::updateAvailable,
            this, &MainWindow::onUpdateAvailable);
    QTimer::singleShot(3000, m_updateChecker, &UpdateChecker::checkNow);

    // ── Initial view ────────────────────────────────────────────────────
    QStringList recent = SettingsDialog::loadRecentProjects();
    m_welcome->setRecentProjects(recent);

    QString compDir = SettingsDialog::loadCompilerDir();
    if (compDir.isEmpty() && recent.isEmpty()) {
        showWelcome();
        QTimer::singleShot(0, this, &MainWindow::checkFirstRun);
    } else if (recent.isEmpty()) {
        showWelcome();
    } else {
        openProject(recent.first());
    }
}

MainWindow::~MainWindow()
{
    if (m_pluginMgr) m_pluginMgr->shutdownAll();
}

void MainWindow::setupShortcuts()
{
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this, this, &MainWindow::doBuild);
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, this, &MainWindow::doRun);
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), this, this, &MainWindow::showCommandPalette);
}

void MainWindow::setStatusMessage(const QString &msg) { statusBar()->showMessage(msg); }

void MainWindow::doBuild()
{
    showWorkspace();
    m_tabs->setCurrentWidget(m_buildTab);
    m_buildTab->build();
}

void MainWindow::doRun()
{
    showWorkspace();
    m_tabs->setCurrentWidget(m_buildTab);
    m_buildTab->runProgram();
}

void MainWindow::toggleTheme()
{
    m_themeMode = (m_themeMode == ThemeMode::Dark) ? ThemeMode::Light : ThemeMode::Dark;
    m_colors = (m_themeMode == ThemeMode::Dark) ? darkColors() : lightColors();
    applyTheme();
    emit themeChanged(m_themeMode, m_colors);
}

void MainWindow::openSettings()
{
    SettingsDialog dlg(m_colors, false, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString newTheme = SettingsDialog::loadEditorTheme();
        ThemeColors newColors = colorsForThemeName(newTheme);
        if (newColors.bgDark != m_colors.bgDark) {
            m_colors = newColors;
            applyTheme();
            emit themeChanged(m_themeMode, m_colors);
        }
    }
}

void MainWindow::checkFirstRun()
{
    if (SettingsDialog::loadCompilerDir().isEmpty()) {
        runQuickSetup();
    }
}

void MainWindow::runQuickSetup()
{
    SetupWizard wiz(m_colors, this);
    if (wiz.exec() == QDialog::Accepted) {
        // Open new project tab after setup completes
        onNewProject();
    }
}

void MainWindow::onNewProject()    { showWorkspace(); m_tabs->setCurrentWidget(m_newProjectTab); }
void MainWindow::onOpenProject()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Open UniLogic Project");
    if (!dir.isEmpty()) openProject(dir);
}

void MainWindow::openProject(const QString &dir)
{
    m_currentProject = dir;
    SettingsDialog::addRecentProject(dir);
    m_fileTree->setRootPath(dir);
    m_buildTab->setProjectDir(dir);
    m_outputPanel->setProjectDir(dir);
    showWorkspace();
    updateTitleBar();
    updateRecentMenu();
    m_welcome->setRecentProjects(SettingsDialog::loadRecentProjects());
    m_outputPanel->loadProfile(dir);
    setStatusMessage(QStringLiteral("Opened project: %1").arg(QDir(dir).dirName()));
}

void MainWindow::openFileInEditor(const QString &filePath)
{
    showWorkspace();
    m_editorTab->openFile(filePath);
    m_tabs->setCurrentWidget(m_editorTab);
}

void MainWindow::goToFileAndLine(const QString &filePath, int line)
{
    showWorkspace();
    m_editorTab->goToFileAndLine(filePath, line);
    m_tabs->setCurrentWidget(m_editorTab);
}

void MainWindow::openRecentProject()
{
    auto *a = qobject_cast<QAction*>(sender());
    if (a) openProject(a->data().toString());
}

// ── Task 5: Multi-window ────────────────────────────────────────────────
void MainWindow::newWindow()
{
    auto *w = new MainWindow;
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->resize(1000, 700);
    if (auto *screen = QApplication::primaryScreen()) {
        QRect geo = screen->geometry();
        w->move((geo.width() - 1000) / 2 + 40, (geo.height() - 700) / 2 + 40);
    }
    w->show();
}

// ── Task 6: Export ──────────────────────────────────────────────────────
void MainWindow::exportProject()
{
    if (m_currentProject.isEmpty()) return;
    QString compDir = SettingsDialog::loadCompilerDir();
    if (compDir.isEmpty()) return;
    ExportDialog dlg(m_currentProject, compDir, m_colors, this);
    dlg.exec();
}

// ── Task 7: Documentation ──────────────────────────────────────────────
void MainWindow::openDocumentation()
{
    QDesktopServices::openUrl(QUrl("https://albazzaztariq.github.io/UniLogic/"));
}

// ── Task 8: Update checker ─────────────────────────────────────────────
void MainWindow::checkForUpdates()
{
    setStatusMessage("Checking for updates...");
    m_updateChecker->checkNow();
}

void MainWindow::onUpdateAvailable(const QString &ver, const QString &url)
{
    m_updateBar->setStyleSheet(QStringLiteral("QFrame { background-color: %1; }").arg(m_colors.navy));
    m_updateLbl->setText(QStringLiteral("UniLogic %1 is available. <a href=\"%2\" style=\"color: white;\">Download now.</a>")
        .arg(ver, url));
    m_updateLbl->setOpenExternalLinks(true);
    m_updateBar->setVisible(true);
    setStatusMessage(QStringLiteral("Update available: v%1").arg(ver));
}

void MainWindow::showDRDiff()
{
    if (m_currentProject.isEmpty()) return;
    DRDiffViewer dlg(m_currentProject, m_colors, this);
    connect(&dlg, &DRDiffViewer::cellClicked, this, &MainWindow::openFileInEditor);
    dlg.exec();
}

void MainWindow::showNormDiff()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select .ul file", m_currentProject, "UniLogic (*.ul)");
    if (filePath.isEmpty()) return;
    QString compDir = SettingsDialog::loadCompilerDir();
    if (compDir.isEmpty()) return;
    NormDiffViewer dlg(filePath, compDir, m_colors, this);
    dlg.exec();
}

void MainWindow::showProjectStats()
{
    if (m_currentProject.isEmpty()) return;
    ProjectStatsDialog dlg(m_currentProject, m_colors, this);
    dlg.exec();
}

void MainWindow::showCommandPalette()
{
    QVector<PaletteCommand> cmds = {
        {"Build",              "Ctrl+B",     [this]() { doBuild(); }},
        {"Run",                "Ctrl+R",     [this]() { doRun(); }},
        {"New Project",        "",           [this]() { onNewProject(); }},
        {"Open Project",       "",           [this]() { onOpenProject(); }},
        {"New Window",         "",           [this]() { newWindow(); }},
        {"Export Project",     "",           [this]() { exportProject(); }},
        {"Settings",           "Ctrl+,",     [this]() { openSettings(); }},
        {"DR Diff",            "",           [this]() { showDRDiff(); }},
        {"Project Statistics", "",           [this]() { showProjectStats(); }},
        {"Show Normalization", "",           [this]() { showNormDiff(); }},
        {"REPL",               "",           [this]() { showWorkspace(); m_tabs->setCurrentWidget(m_replTab); }},
        {"Editor",             "",           [this]() { showWorkspace(); m_tabs->setCurrentWidget(m_editorTab); }},
        {"Documentation",      "",           [this]() { openDocumentation(); }},
        {"Keyboard Shortcuts", "",           [this]() { showShortcuts(); }},
        {"Check for Updates",  "",           [this]() { checkForUpdates(); }},
        {"About",              "",           [this]() { showAbout(); }},
    };
    auto *palette = new CommandPalette(cmds, m_colors, this);
    QPoint center = mapToGlobal(rect().center());
    palette->move(center.x() - 250, center.y() - 150);
    palette->show();
}

void MainWindow::showPlugins()
{
    auto plugins = m_pluginMgr->loadedPlugins();
    QString msg = plugins.isEmpty()
        ? "No plugins loaded.\n\nPlugins are DLLs placed in:\n%APPDATA%\\UniLogic\\plugins\\"
        : "Loaded plugins:\n\n";
    for (const auto &p : plugins)
        msg += QStringLiteral("%1 v%2\n").arg(p.first, p.second);
    QMessageBox::information(this, "Plugins", msg);
}

void MainWindow::showShortcuts() { ShortcutsDialog dlg(m_colors, this); dlg.exec(); }
void MainWindow::showAbout()     { AboutDialog dlg(m_colors, this); dlg.exec(); }
void MainWindow::showWelcome()   { m_stack->setCurrentIndex(0); }
void MainWindow::showWorkspace() { m_stack->setCurrentIndex(1); }

void MainWindow::updateRecentMenu()
{
    m_recentMenu->clear();
    QStringList recent = SettingsDialog::loadRecentProjects();
    if (recent.isEmpty()) { m_recentMenu->addAction("(none)")->setEnabled(false); return; }
    for (const QString &path : recent) {
        QAction *a = m_recentMenu->addAction(QDir::toNativeSeparators(path));
        a->setData(path);
        connect(a, &QAction::triggered, this, &MainWindow::openRecentProject);
    }
}

void MainWindow::updateTitleBar()
{
    if (m_currentProject.isEmpty())
        setWindowTitle("UniLogic Project Manager");
    else
        setWindowTitle(QStringLiteral("%1 \u2014 UniLogic Project Manager")
                           .arg(QDir(m_currentProject).dirName()));
}

void MainWindow::applyTheme()
{
    qApp->setStyleSheet(buildStylesheet(m_colors));
    m_headerBar->setStyleSheet(headerBarStyle(m_colors));
    m_versionLbl->setStyleSheet(
        QStringLiteral("color: %1; font-size: 10pt; background: transparent;").arg(m_colors.versionColor));
    m_settingsBtn->setStyleSheet(themeBtnStyle());
    m_themeBtn->setStyleSheet(themeBtnStyle());
    m_themeBtn->setText((m_themeMode == ThemeMode::Dark) ? QStringLiteral("\u2600") : QStringLiteral("\u263D"));
}
