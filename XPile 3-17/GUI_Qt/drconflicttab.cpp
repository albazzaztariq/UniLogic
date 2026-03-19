// drconflicttab.cpp -- "DR Conflicts" tab implementation

#include "drconflicttab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QSet>
#include <QMap>

static const QStringList DR_SETTING_NAMES = {
    "memory", "safety", "types", "int_width", "concurrency"
};

static QMap<QString, QString> parseDrBlock(const QString &text)
{
    QMap<QString, QString> found;
    static QRegularExpression rx(R"((?://\s*)?@dr\s+(\w+)\s*=\s*(\w+))");
    auto it = rx.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        found[m.captured(1)] = m.captured(2);
    }
    return found;
}

DRConflictTab::DRConflictTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    // ── Header ──────────────────────────────────────────────────────────
    m_titleLbl = new QLabel("DR Conflict Checker");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    layout->addWidget(m_titleLbl);

    m_subtitleLbl = new QLabel(
        "Scan all .ul files in a project for conflicting DR settings.");
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    layout->addWidget(m_subtitleLbl);

    // ── Folder selector ─────────────────────────────────────────────────
    auto *folderRow = new QHBoxLayout;
    auto *folderLbl = new QLabel("Project Folder:");
    folderLbl->setStyleSheet("font-weight: bold;");
    folderRow->addWidget(folderLbl);

    m_folderEdit = new QLineEdit;
    folderRow->addWidget(m_folderEdit, 1);

    auto *browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &DRConflictTab::browse);
    folderRow->addWidget(browseBtn);

    auto *scanBtn = new QPushButton("Scan");
    scanBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(scanBtn, &QPushButton::clicked, this, &DRConflictTab::scan);
    folderRow->addWidget(scanBtn);

    layout->addLayout(folderRow);

    // ── Results tree ────────────────────────────────────────────────────
    auto *resultsGroup  = new QGroupBox("Results");
    auto *resultsLayout = new QVBoxLayout(resultsGroup);

    m_tree = new QTreeWidget;

    QStringList headers = {"File"};
    for (const QString &k : DR_SETTING_NAMES) {
        QString h = k;
        h.replace('_', ' ');
        // Title case
        for (int i = 0; i < h.size(); ++i) {
            if (i == 0 || h[i - 1] == ' ')
                h[i] = h[i].toUpper();
        }
        headers << h;
    }
    m_tree->setHeaderLabels(headers);
    m_tree->setAlternatingRowColors(true);
    m_tree->setRootIsDecorated(false);

    auto *hdr = m_tree->header();
    hdr->setStretchLastSection(true);
    hdr->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < headers.size(); ++i)
        hdr->setSectionResizeMode(i, QHeaderView::ResizeToContents);

    resultsLayout->addWidget(m_tree);
    layout->addWidget(resultsGroup, 1);

    // ── Summary ─────────────────────────────────────────────────────────
    m_summaryLbl = new QLabel;
    m_summaryLbl->setWordWrap(true);
    layout->addWidget(m_summaryLbl);
}

void DRConflictTab::onThemeChanged(ThemeMode /*mode*/, const ThemeColors &colors)
{
    m_colors = colors;
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
}

void DRConflictTab::browse()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select project folder");
    if (!path.isEmpty())
        m_folderEdit->setText(QDir::toNativeSeparators(path));
}

void DRConflictTab::scan()
{
    QString folder = m_folderEdit->text().trimmed();
    if (folder.isEmpty() || !QDir(folder).exists()) {
        QMessageBox::warning(this, "Invalid Folder",
                             "Select a valid project folder.");
        return;
    }

    m_tree->clear();

    // Collect all .ul files recursively
    QStringList ulFiles;
    QDirIterator it(folder, {"*.ul"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        ulFiles << it.next();
    ulFiles.sort();

    if (ulFiles.isEmpty()) {
        m_summaryLbl->setText("No .ul files found in the selected folder.");
        m_summaryLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.orange));
        return;
    }

    // Parse DR settings from each file
    struct FileSettings {
        QString relPath;
        QMap<QString, QString> dr;
    };
    QVector<FileSettings> allSettings;
    QMap<QString, QSet<QString>> valueSets;
    for (const QString &k : DR_SETTING_NAMES)
        valueSets[k] = {};

    QDir baseDir(folder);
    for (const QString &filePath : ulFiles) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QString text = QTextStream(&file).readAll();
        file.close();

        auto dr = parseDrBlock(text);
        QString rel = baseDir.relativeFilePath(filePath);
        allSettings.append({rel, dr});

        for (auto dit = dr.cbegin(); dit != dr.cend(); ++dit) {
            if (valueSets.contains(dit.key()))
                valueSets[dit.key()].insert(dit.value());
        }
    }

    // Determine which settings have conflicts
    QSet<QString> conflicts;
    for (auto vit = valueSets.cbegin(); vit != valueSets.cend(); ++vit) {
        if (vit.value().size() > 1)
            conflicts.insert(vit.key());
    }

    // Populate tree
    for (const auto &fs : allSettings) {
        QStringList values = {fs.relPath};
        bool hasConflict = false;

        for (const QString &key : DR_SETTING_NAMES) {
            QString val = fs.dr.value(key, QStringLiteral("\u2014"));
            values << val;
            if (conflicts.contains(key) && val != QStringLiteral("\u2014"))
                hasConflict = true;
        }

        auto *item = new QTreeWidgetItem(values);
        if (hasConflict) {
            QColor red(m_colors.red);
            for (int col = 0; col < values.size(); ++col)
                item->setForeground(col, red);
        }
        m_tree->addTopLevelItem(item);
    }

    // Summary
    int nFiles  = ulFiles.size();
    int nWithDr = 0;
    for (const auto &fs : allSettings) {
        if (!fs.dr.isEmpty())
            ++nWithDr;
    }

    if (!conflicts.isEmpty()) {
        QStringList clist(conflicts.begin(), conflicts.end());
        clist.sort();
        m_summaryLbl->setText(
            QStringLiteral("Scanned %1 files (%2 with DR headers). CONFLICTS in: %3")
                .arg(nFiles).arg(nWithDr).arg(clist.join(", ")));
        m_summaryLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.red));
    } else {
        m_summaryLbl->setText(
            QStringLiteral("Scanned %1 files (%2 with DR headers). No conflicts found.")
                .arg(nFiles).arg(nWithDr));
        m_summaryLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.green));
    }
}
