// drdiffviewer.cpp -- DR Diff viewer implementation

#include "drdiffviewer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QRegularExpression>
#include <QMap>
#include <QSet>

static const QStringList DR_KEYS = {"memory", "safety", "types", "int_width", "concurrency"};

static QMap<QString, QString> parseDR(const QString &text)
{
    QMap<QString, QString> found;
    static QRegularExpression rx(R"((?://\s*)?@dr\s+(\w+)\s*=\s*(\w+))");
    auto it = rx.globalMatch(text);
    while (it.hasNext()) { auto m = it.next(); found[m.captured(1)] = m.captured(2); }
    return found;
}

DRDiffViewer::DRDiffViewer(const QString &projectDir,
                            const ThemeColors &colors, QWidget *parent)
    : QDialog(parent), m_colors(colors), m_projectDir(projectDir)
{
    setWindowTitle("DR Diff Viewer");
    setMinimumSize(700, 400);
    resize(800, 500);

    auto *layout = new QVBoxLayout(this);

    m_tree = new QTreeWidget;
    QStringList headers = {"File"};
    for (const QString &k : DR_KEYS) {
        QString h = k; h.replace('_', ' ');
        for (int i = 0; i < h.size(); ++i)
            if (i == 0 || h[i-1] == ' ') h[i] = h[i].toUpper();
        headers << h;
    }
    m_tree->setHeaderLabels(headers);
    m_tree->setRootIsDecorated(false);
    m_tree->setAlternatingRowColors(true);
    auto *hdr = m_tree->header();
    hdr->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < headers.size(); ++i)
        hdr->setSectionResizeMode(i, QHeaderView::ResizeToContents);

    connect(m_tree, &QTreeWidget::itemClicked, this, &DRDiffViewer::onItemClicked);
    layout->addWidget(m_tree);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *exportBtn = new QPushButton("Export CSV");
    exportBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(exportBtn, &QPushButton::clicked, this, &DRDiffViewer::exportCsv);
    btnRow->addWidget(exportBtn);
    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    // Scan files
    struct FileDR { QString relPath; QString absPath; QMap<QString,QString> dr; };
    QVector<FileDR> all;
    QMap<QString, QMap<QString, int>> valueCounts; // key -> value -> count

    QDirIterator it(projectDir, {"*.ul"}, QDir::Files, QDirIterator::Subdirectories);
    QDir base(projectDir);
    while (it.hasNext()) {
        QString fp = it.next();
        QFile f(fp);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        auto dr = parseDR(QTextStream(&f).readAll());
        f.close();
        all.append({base.relativeFilePath(fp), fp, dr});
        for (auto di = dr.cbegin(); di != dr.cend(); ++di)
            if (DR_KEYS.contains(di.key()))
                valueCounts[di.key()][di.value()]++;
    }

    // Find majority value for each key
    QMap<QString, QString> majority;
    for (const QString &key : DR_KEYS) {
        QString best; int bestCount = 0;
        for (auto vi = valueCounts[key].cbegin(); vi != valueCounts[key].cend(); ++vi)
            if (vi.value() > bestCount) { best = vi.key(); bestCount = vi.value(); }
        majority[key] = best;
    }

    // Populate tree
    for (const auto &fd : all) {
        QStringList vals = {fd.relPath};
        for (const QString &key : DR_KEYS)
            vals << fd.dr.value(key, QStringLiteral("\u2014"));

        auto *item = new QTreeWidgetItem(vals);
        item->setData(0, Qt::UserRole, fd.absPath);

        for (int i = 0; i < DR_KEYS.size(); ++i) {
            QString val = fd.dr.value(DR_KEYS[i]);
            if (!val.isEmpty() && val != majority[DR_KEYS[i]])
                item->setForeground(i + 1, QColor(m_colors.red));
        }
        m_tree->addTopLevelItem(item);
    }
}

void DRDiffViewer::onItemClicked(QTreeWidgetItem *item, int)
{
    QString path = item->data(0, Qt::UserRole).toString();
    if (!path.isEmpty()) emit cellClicked(path);
}

void DRDiffViewer::exportCsv()
{
    QString path = QFileDialog::getSaveFileName(this, "Export CSV", "", "CSV (*.csv)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);

    // Header
    QStringList headers;
    for (int i = 0; i < m_tree->columnCount(); ++i)
        headers << m_tree->headerItem()->text(i);
    out << headers.join(",") << "\n";

    // Rows
    for (int r = 0; r < m_tree->topLevelItemCount(); ++r) {
        auto *item = m_tree->topLevelItem(r);
        QStringList cols;
        for (int c = 0; c < m_tree->columnCount(); ++c)
            cols << "\"" + item->text(c).replace("\"", "\"\"") + "\"";
        out << cols.join(",") << "\n";
    }
    file.close();
}
