// settingsdialog.cpp -- Application settings dialog implementation

#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QFileInfo>

// ── Static helpers ──────────────────────────────────────────────────────

static QSettings appSettings()
{
    return QSettings(SettingsDialog::SETTINGS_ORG, SettingsDialog::SETTINGS_APP);
}

QString SettingsDialog::loadCompilerDir()
{ return appSettings().value(KEY_COMPILER).toString(); }

void SettingsDialog::saveCompilerDir(const QString &dir)
{ appSettings().setValue(KEY_COMPILER, dir); }

QString SettingsDialog::loadPythonPath()
{ return appSettings().value(KEY_PYTHON, "python").toString(); }

QString SettingsDialog::loadDefaultTarget()
{ return appSettings().value(KEY_TARGET, "C").toString(); }

QString SettingsDialog::loadDefaultDRMemory()
{ return appSettings().value(KEY_DR_MEM, "gc").toString(); }

int SettingsDialog::loadEditorFontSize()
{ return appSettings().value(KEY_FONTSIZE, 12).toInt(); }

QString SettingsDialog::loadEditorTheme()
{ return appSettings().value(KEY_THEME, "Dark Navy").toString(); }

bool SettingsDialog::loadAutoSaveOnBuild()
{ return appSettings().value(KEY_AUTOSAVE, false).toBool(); }

bool SettingsDialog::loadShowWelcome()
{ return appSettings().value(KEY_WELCOME, true).toBool(); }

QStringList SettingsDialog::loadRecentProjects()
{ return appSettings().value(KEY_RECENT).toStringList(); }

void SettingsDialog::addRecentProject(const QString &path)
{
    auto s = appSettings();
    QStringList list = s.value(KEY_RECENT).toStringList();
    list.removeAll(path);
    list.prepend(path);
    while (list.size() > 5) list.removeLast();
    s.setValue(KEY_RECENT, list);
}

// ── Constructor ─────────────────────────────────────────────────────────

SettingsDialog::SettingsDialog(const ThemeColors &colors, bool firstRun,
                               QWidget *parent)
    : QDialog(parent), m_colors(colors)
{
    setWindowTitle("Settings");
    setMinimumWidth(560);
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    if (firstRun) {
        m_msgLbl = new QLabel(
            "Welcome to UniLogic Project Manager.\n"
            "Set your compiler path to get started.");
        m_msgLbl->setStyleSheet(
            QStringLiteral("color: %1; font-size: 11pt; font-weight: bold;")
                .arg(m_colors.navyLight));
        m_msgLbl->setWordWrap(true);
        layout->addWidget(m_msgLbl);
    }

    // ── Compiler group ──────────────────────────────────────────────────
    auto *compGroup = new QGroupBox("Compiler");
    auto *compGrid  = new QGridLayout(compGroup);
    compGrid->setColumnStretch(1, 1);
    compGrid->setVerticalSpacing(8);
    compGrid->setHorizontalSpacing(12);

    // Compiler path
    compGrid->addWidget(new QLabel("Compiler Path:"), 0, 0);
    auto *compRow = new QHBoxLayout;
    m_compilerEdit = new QLineEdit;
    m_compilerEdit->setPlaceholderText("Path to XPile directory containing Main.py");
    compRow->addWidget(m_compilerEdit);
    auto *compBrowse = new QPushButton("Browse...");
    connect(compBrowse, &QPushButton::clicked, this, &SettingsDialog::browseCompilerDir);
    compRow->addWidget(compBrowse);
    compGrid->addLayout(compRow, 0, 1);

    // Python path
    compGrid->addWidget(new QLabel("Python Path:"), 1, 0);
    auto *pyRow = new QHBoxLayout;
    m_pythonEdit = new QLineEdit;
    m_pythonEdit->setPlaceholderText("python");
    pyRow->addWidget(m_pythonEdit);
    auto *pyBrowse = new QPushButton("Browse...");
    connect(pyBrowse, &QPushButton::clicked, this, &SettingsDialog::browsePythonPath);
    pyRow->addWidget(pyBrowse);
    compGrid->addLayout(pyRow, 1, 1);

    // Default target
    compGrid->addWidget(new QLabel("Default Target:"), 2, 0);
    m_targetCombo = new QComboBox;
    m_targetCombo->addItems({"C", "Python", "JavaScript", "LLVM IR", "WASM"});
    compGrid->addWidget(m_targetCombo, 2, 1);

    // Default DR memory
    compGrid->addWidget(new QLabel("Default DR Memory:"), 3, 0);
    m_drMemCombo = new QComboBox;
    m_drMemCombo->addItems({"gc", "manual", "refcount", "arena"});
    compGrid->addWidget(m_drMemCombo, 3, 1);

    layout->addWidget(compGroup);

    // ── Editor group ────────────────────────────────────────────────────
    auto *edGroup = new QGroupBox("Editor");
    auto *edGrid  = new QGridLayout(edGroup);
    edGrid->setColumnStretch(1, 1);
    edGrid->setVerticalSpacing(8);
    edGrid->setHorizontalSpacing(12);

    edGrid->addWidget(new QLabel("Font Size:"), 0, 0);
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setSuffix(" pt");
    edGrid->addWidget(m_fontSizeSpin, 0, 1);

    edGrid->addWidget(new QLabel("Editor Theme:"), 1, 0);
    m_themeCombo = new QComboBox;
    m_themeCombo->addItems({"Dark Navy", "Light", "High Contrast", "Solarized Dark", "Monokai"});
    edGrid->addWidget(m_themeCombo, 1, 1);

    m_autoSaveCheck = new QCheckBox("Auto-save all open files before building");
    edGrid->addWidget(m_autoSaveCheck, 2, 0, 1, 2);

    layout->addWidget(edGroup);

    // ── UI group ────────────────────────────────────────────────────────
    auto *uiGroup = new QGroupBox("User Interface");
    auto *uiLayout = new QVBoxLayout(uiGroup);
    m_welcomeCheck = new QCheckBox("Show welcome screen on startup");
    uiLayout->addWidget(m_welcomeCheck);
    layout->addWidget(uiGroup);

    layout->addStretch();

    // ── Buttons ─────────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton("Cancel");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto *saveBtn = new QPushButton("Save");
    saveBtn->setStyleSheet(navyBtnStyle(m_colors));
    saveBtn->setDefault(true);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::accept);
    btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow);

    // ── Load current values ─────────────────────────────────────────────
    auto s = appSettings();
    m_compilerEdit->setText(QDir::toNativeSeparators(s.value(KEY_COMPILER).toString()));
    m_pythonEdit->setText(s.value(KEY_PYTHON, "python").toString());

    int tIdx = m_targetCombo->findText(s.value(KEY_TARGET, "C").toString());
    if (tIdx >= 0) m_targetCombo->setCurrentIndex(tIdx);

    int mIdx = m_drMemCombo->findText(s.value(KEY_DR_MEM, "gc").toString());
    if (mIdx >= 0) m_drMemCombo->setCurrentIndex(mIdx);

    m_fontSizeSpin->setValue(s.value(KEY_FONTSIZE, 12).toInt());

    int thIdx = m_themeCombo->findText(s.value(KEY_THEME, "Dark Navy").toString());
    if (thIdx >= 0) m_themeCombo->setCurrentIndex(thIdx);

    m_autoSaveCheck->setChecked(s.value(KEY_AUTOSAVE, false).toBool());
    m_welcomeCheck->setChecked(s.value(KEY_WELCOME, true).toBool());
}

QString SettingsDialog::compilerDir() const
{ return m_compilerEdit->text().trimmed(); }

void SettingsDialog::setCompilerDir(const QString &dir)
{ m_compilerEdit->setText(QDir::toNativeSeparators(dir)); }

void SettingsDialog::browseCompilerDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select XPile compiler directory (contains Main.py)");
    if (!dir.isEmpty())
        m_compilerEdit->setText(QDir::toNativeSeparators(dir));
}

void SettingsDialog::browsePythonPath()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Select Python executable", QString(),
        "Executable (*.exe);;All Files (*)");
    if (!path.isEmpty())
        m_pythonEdit->setText(QDir::toNativeSeparators(path));
}

void SettingsDialog::accept()
{
    QString dir = compilerDir();
    if (!dir.isEmpty()) {
        QString mainPy = QDir(dir).absoluteFilePath("Main.py");
        if (!QFileInfo::exists(mainPy)) {
            QMessageBox::warning(this, "Main.py Not Found",
                QStringLiteral("Could not find Main.py in:\n%1")
                    .arg(QDir::toNativeSeparators(dir)));
            return;
        }
    }

    auto s = appSettings();
    s.setValue(KEY_COMPILER, dir);
    s.setValue(KEY_PYTHON,   m_pythonEdit->text().trimmed());
    s.setValue(KEY_TARGET,   m_targetCombo->currentText());
    s.setValue(KEY_DR_MEM,   m_drMemCombo->currentText());
    s.setValue(KEY_FONTSIZE, m_fontSizeSpin->value());
    s.setValue(KEY_THEME,    m_themeCombo->currentText());
    s.setValue(KEY_AUTOSAVE, m_autoSaveCheck->isChecked());
    s.setValue(KEY_WELCOME,  m_welcomeCheck->isChecked());

    QDialog::accept();
}
