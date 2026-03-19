// setupwizard.cpp -- First-run setup wizard implementation

#include "setupwizard.h"
#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QSettings>
#include <QStandardPaths>

SetupWizard::SetupWizard(const ThemeColors &colors, QWidget *parent)
    : QDialog(parent), m_colors(colors)
{
    setWindowTitle("UniLogic Quick Setup");
    setMinimumSize(550, 380);
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto *titleLbl = new QLabel(QStringLiteral(
        "\u221e Welcome to UniLogic"));
    titleLbl->setStyleSheet(QStringLiteral(
        "font-size: 16pt; font-weight: bold; color: %1;").arg(colors.navyLight));
    layout->addWidget(titleLbl);

    m_pages = new QStackedWidget;

    // ── Page 0: Find Python ─────────────────────────────────────────────
    auto *pyPage = new QWidget;
    auto *pyLayout = new QVBoxLayout(pyPage);

    auto *pyTitle = new QLabel("Step 1: Find Python");
    pyTitle->setStyleSheet("font-size: 12pt; font-weight: bold;");
    pyLayout->addWidget(pyTitle);
    pyLayout->addWidget(new QLabel("UniLogic needs Python 3.10+ to compile .ul files."));

    auto *pyRow = new QHBoxLayout;
    m_pythonEdit = new QLineEdit("python");
    m_pythonEdit->setPlaceholderText("Path to python executable");
    m_pythonEdit->setToolTip("The Python interpreter used to run the compiler");
    pyRow->addWidget(m_pythonEdit, 1);
    auto *pyBrowse = new QPushButton("Browse...");
    pyBrowse->setToolTip("Browse for python.exe");
    connect(pyBrowse, &QPushButton::clicked, this, &SetupWizard::browsePython);
    pyRow->addWidget(pyBrowse);
    auto *pyTest = new QPushButton("Test");
    pyTest->setStyleSheet(navyBtnStyle(colors));
    pyTest->setToolTip("Verify Python works");
    connect(pyTest, &QPushButton::clicked, this, &SetupWizard::testPython);
    pyRow->addWidget(pyTest);
    pyLayout->addLayout(pyRow);

    m_pythonStatus = new QLabel;
    m_pythonStatus->setWordWrap(true);
    pyLayout->addWidget(m_pythonStatus);
    pyLayout->addStretch();

    m_pages->addWidget(pyPage);

    // ── Page 1: Find Compiler ───────────────────────────────────────────
    auto *compPage = new QWidget;
    auto *compLayout = new QVBoxLayout(compPage);

    auto *compTitle = new QLabel("Step 2: Find Compiler");
    compTitle->setStyleSheet("font-size: 12pt; font-weight: bold;");
    compLayout->addWidget(compTitle);
    compLayout->addWidget(new QLabel("Select the XPile directory containing Main.py."));

    auto *compRow = new QHBoxLayout;
    m_compEdit = new QLineEdit;
    m_compEdit->setPlaceholderText("Path to XPile directory");
    m_compEdit->setToolTip("The directory containing Main.py");
    compRow->addWidget(m_compEdit, 1);
    auto *compBrowse = new QPushButton("Browse...");
    compBrowse->setToolTip("Browse for the XPile compiler directory");
    connect(compBrowse, &QPushButton::clicked, this, &SetupWizard::browseCompiler);
    compRow->addWidget(compBrowse);
    auto *compTest = new QPushButton("Test");
    compTest->setStyleSheet(navyBtnStyle(colors));
    compTest->setToolTip("Verify Main.py exists and works");
    connect(compTest, &QPushButton::clicked, this, &SetupWizard::testCompiler);
    compRow->addWidget(compTest);
    compLayout->addLayout(compRow);

    m_compStatus = new QLabel;
    m_compStatus->setWordWrap(true);
    compLayout->addWidget(m_compStatus);
    compLayout->addStretch();

    m_pages->addWidget(compPage);

    // ── Page 2: Done ────────────────────────────────────────────────────
    auto *donePage = new QWidget;
    auto *doneLayout = new QVBoxLayout(donePage);
    m_doneLbl = new QLabel(
        "Setup complete!\n\n"
        "Click Finish to create your first project.");
    m_doneLbl->setStyleSheet(QStringLiteral(
        "font-size: 12pt; color: %1;").arg(colors.green));
    m_doneLbl->setWordWrap(true);
    doneLayout->addWidget(m_doneLbl);
    doneLayout->addStretch();

    m_pages->addWidget(donePage);

    layout->addWidget(m_pages, 1);

    // ── Navigation buttons ──────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_backBtn = new QPushButton("Back");
    m_backBtn->setToolTip("Go to previous step");
    connect(m_backBtn, &QPushButton::clicked, this, &SetupWizard::prevStep);
    btnRow->addWidget(m_backBtn);
    m_nextBtn = new QPushButton("Next");
    m_nextBtn->setStyleSheet(navyBtnStyle(colors));
    m_nextBtn->setToolTip("Proceed to next step");
    connect(m_nextBtn, &QPushButton::clicked, this, &SetupWizard::nextStep);
    btnRow->addWidget(m_nextBtn);
    layout->addLayout(btnRow);

    // Auto-detect python
    QProcess detect;
    detect.start("python", {"--version"});
    if (detect.waitForFinished(3000) && detect.exitCode() == 0) {
        QString ver = QString::fromUtf8(detect.readAllStandardOutput()).trimmed();
        m_pythonStatus->setText(QStringLiteral("Auto-detected: %1").arg(ver));
        m_pythonStatus->setStyleSheet(QStringLiteral("color: %1;").arg(colors.green));
    }

    // Auto-detect compiler in common locations
    QStringList searchPaths = {
        QDir::currentPath(),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/UniLogic",
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/XPile",
    };
    for (const QString &p : searchPaths) {
        if (QFileInfo::exists(QDir(p).filePath("Main.py"))) {
            m_compEdit->setText(QDir::toNativeSeparators(p));
            break;
        }
    }

    updateButtons();
}

QString SetupWizard::pythonPath() const  { return m_pythonEdit->text().trimmed(); }
QString SetupWizard::compilerDir() const { return m_compEdit->text().trimmed(); }

void SetupWizard::nextStep()
{
    int page = m_pages->currentIndex();
    if (page == 2) {
        // Finish — save settings
        SettingsDialog::saveCompilerDir(compilerDir());
        QSettings s(SettingsDialog::SETTINGS_ORG, SettingsDialog::SETTINGS_APP);
        s.setValue(SettingsDialog::KEY_PYTHON, pythonPath());
        accept();
        return;
    }
    m_pages->setCurrentIndex(page + 1);
    updateButtons();
}

void SetupWizard::prevStep()
{
    int page = m_pages->currentIndex();
    if (page > 0) m_pages->setCurrentIndex(page - 1);
    updateButtons();
}

void SetupWizard::updateButtons()
{
    int page = m_pages->currentIndex();
    m_backBtn->setVisible(page > 0);
    m_nextBtn->setText(page == 2 ? "Finish" : "Next");
}

void SetupWizard::browsePython()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Select Python executable", QString(),
        "Executable (*.exe);;All Files (*)");
    if (!path.isEmpty())
        m_pythonEdit->setText(QDir::toNativeSeparators(path));
}

void SetupWizard::browseCompiler()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select XPile compiler directory");
    if (!dir.isEmpty())
        m_compEdit->setText(QDir::toNativeSeparators(dir));
}

void SetupWizard::testPython()
{
    QProcess proc;
    proc.start(m_pythonEdit->text().trimmed(), {"--version"});
    if (proc.waitForFinished(5000) && proc.exitCode() == 0) {
        QString ver = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        m_pythonStatus->setText(QStringLiteral("OK: %1").arg(ver));
        m_pythonStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.green));
    } else {
        m_pythonStatus->setText(
            "Python not found. Install from https://python.org/downloads/");
        m_pythonStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.red));
    }
}

void SetupWizard::testCompiler()
{
    QString dir = m_compEdit->text().trimmed();
    QString mainPy = QDir(dir).filePath("Main.py");
    if (!QFileInfo::exists(mainPy)) {
        m_compStatus->setText("Main.py not found in this directory.");
        m_compStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.red));
        return;
    }
    QProcess proc;
    proc.start(m_pythonEdit->text().trimmed(), {mainPy, "--version"});
    if (proc.waitForFinished(5000)) {
        QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        if (out.isEmpty()) out = "Main.py found";
        m_compStatus->setText(QStringLiteral("OK: %1").arg(out));
        m_compStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.green));
    } else {
        m_compStatus->setText("Main.py found but could not run it.");
        m_compStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.orange));
    }
}
