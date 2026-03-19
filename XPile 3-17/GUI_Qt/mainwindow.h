#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QSplitter>
#include <QStackedWidget>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include "theme.h"

class NewProjectTab;
class DRConfiguratorTab;
class BuildTab;
class DRConflictTab;
class PackagesTab;
class EditorTab;
class ReplTab;
class FileTreePanel;
class WelcomeScreen;
class OutputPanel;
class PluginManager;
class UpdateChecker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ThemeMode currentTheme() const { return m_themeMode; }
    ThemeColors currentColors() const { return m_colors; }
    QString currentProject() const { return m_currentProject; }

signals:
    void themeChanged(ThemeMode mode, const ThemeColors &colors);

public slots:
    void openProject(const QString &dir);
    void openFileInEditor(const QString &filePath);
    void goToFileAndLine(const QString &filePath, int line);
    void setStatusMessage(const QString &msg);

private slots:
    void toggleTheme();
    void openSettings();
    void onNewProject();
    void onOpenProject();
    void openRecentProject();
    void newWindow();
    void exportProject();
    void openDocumentation();
    void checkForUpdates();
    void onUpdateAvailable(const QString &ver, const QString &url);
    void showDRDiff();
    void showNormDiff();
    void showProjectStats();
    void showCommandPalette();
    void showPlugins();
    void showShortcuts();
    void showAbout();
    void doBuild();
    void doRun();
    void runQuickSetup();

private:
    void applyTheme();
    void checkFirstRun();
    void showWelcome();
    void showWorkspace();
    void updateRecentMenu();
    void updateTitleBar();
    void setupShortcuts();

    ThemeMode     m_themeMode = ThemeMode::Dark;
    ThemeColors   m_colors;
    QString       m_currentProject;

    QFrame       *m_headerBar    = nullptr;
    QLabel       *m_titleLbl     = nullptr;
    QLabel       *m_versionLbl   = nullptr;
    QPushButton  *m_settingsBtn  = nullptr;
    QPushButton  *m_themeBtn     = nullptr;
    QFrame       *m_updateBar    = nullptr;
    QLabel       *m_updateLbl    = nullptr;

    QMenu        *m_fileMenu     = nullptr;
    QMenu        *m_recentMenu   = nullptr;
    QMenu        *m_toolsMenu    = nullptr;
    QMenu        *m_analysisMenu = nullptr;
    QMenu        *m_helpMenu     = nullptr;

    QStackedWidget *m_stack       = nullptr;
    WelcomeScreen  *m_welcome     = nullptr;
    QSplitter      *m_outerSplit  = nullptr;
    QSplitter      *m_splitter    = nullptr;
    FileTreePanel  *m_fileTree    = nullptr;
    QTabWidget     *m_tabs        = nullptr;
    OutputPanel    *m_outputPanel = nullptr;

    NewProjectTab      *m_newProjectTab = nullptr;
    DRConfiguratorTab  *m_drConfigTab   = nullptr;
    BuildTab           *m_buildTab      = nullptr;
    DRConflictTab      *m_drConflictTab = nullptr;
    PackagesTab        *m_packagesTab   = nullptr;
    EditorTab          *m_editorTab     = nullptr;
    ReplTab            *m_replTab       = nullptr;

    PluginManager      *m_pluginMgr     = nullptr;
    UpdateChecker      *m_updateChecker = nullptr;
};
