#include "projectstats.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QMap>

ProjectStatsDialog::ProjectStatsDialog(const QString &projectDir,
                                       const ThemeColors &colors, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Project Statistics");
    resize(500, 450);

    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Project Statistics");
    title->setStyleSheet(QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;").arg(colors.navyLight));
    layout->addWidget(title);

    // Scan
    int totalFiles = 0, totalLines = 0, totalFunctions = 0, totalTypes = 0, totalObjects = 0, totalImports = 0;
    QString mostComplexFile; int mostComplexCount = 0;
    QMap<QString, int> memUsage;

    static QRegularExpression funcRx(R"(\bfunction\s+\w+)");
    static QRegularExpression typeRx(R"(\btype\s+\w+)");
    static QRegularExpression objRx(R"(\bobject\s+\w+)");
    static QRegularExpression importRx(R"(\bimport\s+)");
    static QRegularExpression drMemRx(R"(@dr\s+memory\s*=\s*(\w+))");

    QDirIterator it(projectDir, {"*.ul"}, QDir::Files, QDirIterator::Subdirectories);
    QDir base(projectDir);
    while (it.hasNext()) {
        QString fp = it.next();
        QFile file(fp);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QString text = QTextStream(&file).readAll();
        file.close();

        totalFiles++;
        int lines = text.count('\n') + 1;
        totalLines += lines;

        int funcs = 0;
        auto fi2 = funcRx.globalMatch(text); while (fi2.hasNext()) { fi2.next(); funcs++; totalFunctions++; }
        auto ti = typeRx.globalMatch(text); while (ti.hasNext()) { ti.next(); totalTypes++; }
        auto oi = objRx.globalMatch(text); while (oi.hasNext()) { oi.next(); totalObjects++; }
        auto ii = importRx.globalMatch(text); while (ii.hasNext()) { ii.next(); totalImports++; }

        if (funcs > mostComplexCount) { mostComplexCount = funcs; mostComplexFile = base.relativeFilePath(fp); }

        auto mi = drMemRx.globalMatch(text);
        while (mi.hasNext()) { auto m = mi.next(); memUsage[m.captured(1)]++; }
    }

    auto *tree = new QTreeWidget;
    tree->setHeaderLabels({"Metric", "Value"});
    tree->setRootIsDecorated(false);
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    auto addRow = [&](const QString &metric, const QString &value) {
        tree->addTopLevelItem(new QTreeWidgetItem({metric, value}));
    };

    addRow("Total .ul files", QString::number(totalFiles));
    addRow("Total lines of code", QString::number(totalLines));
    addRow("Total functions", QString::number(totalFunctions));
    addRow("Total types", QString::number(totalTypes));
    addRow("Total objects", QString::number(totalObjects));
    addRow("Files with imports", QString::number(totalImports));
    addRow("Most complex file", QStringLiteral("%1 (%2 functions)").arg(mostComplexFile).arg(mostComplexCount));

    for (auto mi = memUsage.cbegin(); mi != memUsage.cend(); ++mi)
        addRow(QStringLiteral("DR memory=%1").arg(mi.key()), QStringLiteral("%1 files").arg(mi.value()));

    layout->addWidget(tree, 1);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
}
