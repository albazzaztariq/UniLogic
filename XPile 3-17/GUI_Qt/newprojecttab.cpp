// newprojecttab.cpp -- "New Project" tab implementation

#include "newprojecttab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

// Standard project directories
static const QStringList STANDARD_DIRS = {
    "Lexer", "Parser", "AST", "Semantic", "Codegen", "REPL", "LSP"
};

NewProjectTab::NewProjectTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    // ── Header ──────────────────────────────────────────────────────────
    m_titleLbl = new QLabel("New UniLogic Project");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    layout->addWidget(m_titleLbl);

    m_subtitleLbl = new QLabel(
        "Creates the standard XPile folder structure with a blank entry point.");
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    layout->addWidget(m_subtitleLbl);

    // ── Form ────────────────────────────────────────────────────────────
    auto *formGroup  = new QGroupBox("Project Settings");
    auto *formLayout = new QGridLayout(formGroup);
    formLayout->setColumnStretch(1, 1);
    formLayout->setVerticalSpacing(10);
    formLayout->setHorizontalSpacing(12);

    // Project Name
    auto *nameLbl = new QLabel("Project Name:");
    nameLbl->setStyleSheet("font-weight: bold;");
    formLayout->addWidget(nameLbl, 0, 0);
    m_nameEdit = new QLineEdit("MyProject");
    formLayout->addWidget(m_nameEdit, 0, 1);

    // Location
    auto *locLbl = new QLabel("Location:");
    locLbl->setStyleSheet("font-weight: bold;");
    formLayout->addWidget(locLbl, 1, 0);

    auto *locRow = new QHBoxLayout;
    m_locEdit = new QLineEdit(
        QDir::toNativeSeparators(
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + "/UniLogic"));
    locRow->addWidget(m_locEdit);

    auto *browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &NewProjectTab::browse);
    locRow->addWidget(browseBtn);
    formLayout->addLayout(locRow, 1, 1);

    // Entry File
    auto *entryLbl = new QLabel("Entry File:");
    entryLbl->setStyleSheet("font-weight: bold;");
    formLayout->addWidget(entryLbl, 2, 0);
    m_entryEdit = new QLineEdit("main.ul");
    formLayout->addWidget(m_entryEdit, 2, 1);

    // DR Preset
    auto *presetLbl = new QLabel("DR Preset:");
    presetLbl->setStyleSheet("font-weight: bold;");
    formLayout->addWidget(presetLbl, 3, 0);
    m_presetCombo = new QComboBox;
    m_presetCombo->addItems({"(none)", "Safe Default", "High Performance", "Embedded"});
    formLayout->addWidget(m_presetCombo, 3, 1);

    layout->addWidget(formGroup);

    // ── Directories ─────────────────────────────────────────────────────
    auto *dirsGroup  = new QGroupBox("Directories to Create");
    auto *dirsLayout = new QGridLayout(dirsGroup);
    dirsLayout->setVerticalSpacing(8);

    for (int i = 0; i < STANDARD_DIRS.size(); ++i) {
        const QString &d = STANDARD_DIRS[i];
        auto *cb = new QCheckBox(d);
        cb->setChecked(true);
        m_dirChecks[d] = cb;
        dirsLayout->addWidget(cb, i / 4, i % 4);
    }
    layout->addWidget(dirsGroup);

    // ── Create button ───────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    m_createBtn = new QPushButton("Create Project");
    m_createBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(m_createBtn, &QPushButton::clicked, this, &NewProjectTab::createProject);
    btnRow->addWidget(m_createBtn);

    m_statusLbl = new QLabel;
    btnRow->addWidget(m_statusLbl);
    btnRow->addStretch();
    layout->addLayout(btnRow);
    layout->addStretch();
}

void NewProjectTab::onThemeChanged(ThemeMode /*mode*/, const ThemeColors &colors)
{
    m_colors = colors;
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    m_createBtn->setStyleSheet(navyBtnStyle(m_colors));
}

void NewProjectTab::browse()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select parent folder");
    if (!path.isEmpty())
        m_locEdit->setText(QDir::toNativeSeparators(path));
}

void NewProjectTab::createProject()
{
    QString name  = m_nameEdit->text().trimmed();
    QString loc   = m_locEdit->text().trimmed();
    QString entry = m_entryEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Missing Name", "Enter a project name.");
        return;
    }
    if (loc.isEmpty()) {
        QMessageBox::warning(this, "Missing Location", "Select a folder location.");
        return;
    }
    if (!entry.endsWith(".ul"))
        entry += ".ul";

    QDir projectDir(loc + "/" + name);
    if (!projectDir.mkpath(".")) {
        m_statusLbl->setText("Error: could not create directory");
        m_statusLbl->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.red));
        return;
    }

    for (auto it = m_dirChecks.cbegin(); it != m_dirChecks.cend(); ++it) {
        if (it.value()->isChecked())
            projectDir.mkpath(it.key());
    }

    QString entryPath = projectDir.filePath(entry);
    QFile file(entryPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << makeEntryContent(name);
        file.close();
    }

    QString absPath = projectDir.absolutePath();
    m_statusLbl->setText(QStringLiteral("Created: %1").arg(
        QDir::toNativeSeparators(absPath)));
    m_statusLbl->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.green));

    emit projectCreated(absPath);
}

QString NewProjectTab::makeEntryContent(const QString &name) const
{
    QStringList lines;
    lines << QStringLiteral("// %1 — UniLogic project entry point\n").arg(name);

    QString preset = m_presetCombo->currentText();

    struct DRPreset { QString key; QString val; };
    QVector<DRPreset> dr;

    if (preset == "Safe Default") {
        dr = {{"memory","gc"}, {"safety","checked"}, {"types","strict"},
              {"int_width","64"}, {"concurrency","threaded"}};
    } else if (preset == "High Performance") {
        dr = {{"memory","manual"}, {"safety","unchecked"}, {"types","strict"},
              {"int_width","64"}, {"concurrency","parallel"}};
    } else if (preset == "Embedded") {
        dr = {{"memory","arena"}, {"safety","checked"}, {"types","strict"},
              {"int_width","32"}, {"concurrency","cooperative"}};
    }

    if (!dr.isEmpty()) {
        lines << "// DR Configuration";
        for (const auto &d : dr)
            lines << QStringLiteral("@dr %1 = %2").arg(d.key, d.val);
        lines << "";
    }

    lines << "function main() returns int";
    lines << "  print 0";
    lines << "  return 0";
    lines << "end function";
    lines << "";

    return lines.join("\n");
}
