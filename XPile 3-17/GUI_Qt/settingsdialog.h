#pragma once
// settingsdialog.h -- Application settings dialog

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "theme.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const ThemeColors &colors,
                            bool firstRun = false,
                            QWidget *parent = nullptr);

    QString compilerDir() const;
    void    setCompilerDir(const QString &dir);

    // QSettings keys used app-wide
    static constexpr const char *SETTINGS_ORG  = "UniLogic";
    static constexpr const char *SETTINGS_APP  = "ProjectManager";
    static constexpr const char *KEY_COMPILER  = "build/compilerDir";
    static constexpr const char *KEY_PYTHON    = "build/pythonPath";
    static constexpr const char *KEY_TARGET    = "build/defaultTarget";
    static constexpr const char *KEY_DR_MEM    = "build/defaultDRMemory";
    static constexpr const char *KEY_FONTSIZE  = "editor/fontSize";
    static constexpr const char *KEY_THEME     = "editor/theme";
    static constexpr const char *KEY_AUTOSAVE  = "editor/autoSaveOnBuild";
    static constexpr const char *KEY_WELCOME   = "ui/showWelcome";
    static constexpr const char *KEY_RECENT    = "recentProjects";

    // Load/save helpers
    static QString loadCompilerDir();
    static void    saveCompilerDir(const QString &dir);

    static QString     loadPythonPath();
    static QString     loadDefaultTarget();
    static QString     loadDefaultDRMemory();
    static int         loadEditorFontSize();
    static QString     loadEditorTheme();
    static bool        loadAutoSaveOnBuild();
    static bool        loadShowWelcome();
    static QStringList loadRecentProjects();
    static void        addRecentProject(const QString &path);

private slots:
    void browseCompilerDir();
    void browsePythonPath();
    void accept() override;

private:
    ThemeColors  m_colors;
    QLabel      *m_msgLbl        = nullptr;
    QLineEdit   *m_compilerEdit  = nullptr;
    QLineEdit   *m_pythonEdit    = nullptr;
    QComboBox   *m_targetCombo   = nullptr;
    QComboBox   *m_drMemCombo    = nullptr;
    QSpinBox    *m_fontSizeSpin  = nullptr;
    QComboBox   *m_themeCombo    = nullptr;
    QCheckBox   *m_autoSaveCheck = nullptr;
    QCheckBox   *m_welcomeCheck  = nullptr;
};
