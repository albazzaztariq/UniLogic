#include "exportdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QScrollBar>
#include <QFont>

ExportDialog::ExportDialog(const QString &projectDir, const QString &compilerDir,
                           const ThemeColors &colors, QWidget *parent)
    : QDialog(parent), m_colors(colors), m_projectDir(projectDir), m_compilerDir(compilerDir)
{
    setWindowTitle("Export Project");
    setMinimumSize(550, 400);

    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Export Project as Distributable");
    title->setStyleSheet(QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;").arg(colors.navyLight));
    layout->addWidget(title);

    auto *grid = new QGridLayout;
    grid->setColumnStretch(1, 1);
    grid->addWidget(new QLabel("Export Format:"), 0, 0);
    m_formatCombo = new QComboBox;
    m_formatCombo->addItems({"C source + Makefile", "Python script + requirements.txt",
                             "JavaScript + package.json", "Native binary", "Inno Setup installer"});
    m_formatCombo->setToolTip("Select the export format for distribution");
    grid->addWidget(m_formatCombo, 0, 1);

    grid->addWidget(new QLabel("Output Directory:"), 1, 0);
    auto *outRow = new QHBoxLayout;
    m_outputEdit = new QLineEdit(QDir(projectDir).filePath("dist"));
    m_outputEdit->setPlaceholderText("Destination directory for export");
    outRow->addWidget(m_outputEdit);
    auto *browseBtn = new QPushButton("Browse...");
    browseBtn->setToolTip("Select output directory");
    connect(browseBtn, &QPushButton::clicked, this, &ExportDialog::browseOutput);
    outRow->addWidget(browseBtn);
    grid->addLayout(outRow, 1, 1);
    layout->addLayout(grid);

    m_exportBtn = new QPushButton("Export");
    m_exportBtn->setStyleSheet(navyBtnStyle(colors));
    m_exportBtn->setToolTip("Start the export process");
    connect(m_exportBtn, &QPushButton::clicked, this, &ExportDialog::doExport);
    layout->addWidget(m_exportBtn);

    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setFont(QFont("Consolas", 9));
    m_log->setPlaceholderText("Export log...");
    layout->addWidget(m_log, 1);
}

void ExportDialog::browseOutput()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory");
    if (!dir.isEmpty()) m_outputEdit->setText(QDir::toNativeSeparators(dir));
}

void ExportDialog::doExport()
{
    m_log->clear();
    m_exportBtn->setEnabled(false);
    QString outDir = m_outputEdit->text().trimmed();
    QDir().mkpath(outDir);

    QStringList targets = {"c", "python", "js", "native", "installer"};
    QString target = targets[m_formatCombo->currentIndex()];

    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    QStringList args = {mainPy, "export", "--target", target, "--output", outDir, m_projectDir};

    m_log->append(QStringLiteral("<span style=\"color:%1;\"> > python %2</span>")
        .arg(m_colors.textDim, args.join(' ').toHtmlEscaped()));

    QProcess proc;
    proc.setWorkingDirectory(m_projectDir);
    proc.start("python", args);
    if (!proc.waitForFinished(120000)) {
        m_log->append(QStringLiteral("<span style=\"color:%1;\">[timeout]</span>").arg(m_colors.red));
        m_exportBtn->setEnabled(true);
        return;
    }

    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    QString err = QString::fromUtf8(proc.readAllStandardError()).trimmed();
    if (!out.isEmpty()) m_log->append(out.toHtmlEscaped());
    if (!err.isEmpty()) m_log->append(QStringLiteral("<span style=\"color:%1;\">%2</span>").arg(m_colors.red, err.toHtmlEscaped()));

    if (proc.exitCode() == 0)
        m_log->append(QStringLiteral("<span style=\"color:%1;\">Export complete: %2</span>")
            .arg(m_colors.green, QDir::toNativeSeparators(outDir)));
    else
        m_log->append(QStringLiteral("<span style=\"color:%1;\">Export failed (exit %2)</span>")
            .arg(m_colors.red).arg(proc.exitCode()));

    m_exportBtn->setEnabled(true);
}
