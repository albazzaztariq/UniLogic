// outputpanel.cpp -- Tabbed output panel implementation

#include "outputpanel.h"
#include "drmonitor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QDateTime>
#include <QFont>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

OutputPanel::OutputPanel(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Toggle bar ──────────────────────────────────────────────────────
    auto *toggleBar = new QHBoxLayout;
    toggleBar->setContentsMargins(8, 2, 8, 2);

    m_toggleBtn = new QPushButton(QStringLiteral("\u25BC Output"));  // ▼
    m_toggleBtn->setStyleSheet(
        QStringLiteral("QPushButton { background: transparent; border: none;"
                        " color: %1; font-weight: bold; font-size: 9pt; padding: 2px 6px; }"
                        "QPushButton:hover { color: %2; }")
            .arg(m_colors.textDim, m_colors.text));
    m_toggleBtn->setCursor(Qt::PointingHandCursor);
    connect(m_toggleBtn, &QPushButton::clicked, this, &OutputPanel::toggleCollapse);
    toggleBar->addWidget(m_toggleBtn);
    toggleBar->addStretch();
    layout->addLayout(toggleBar);

    // ── Tab widget ──────────────────────────────────────────────────────
    m_tabs = new QTabWidget;

    // Build tab
    auto *buildWidget = new QWidget;
    auto *buildLayout = new QVBoxLayout(buildWidget);
    buildLayout->setContentsMargins(0, 0, 0, 0);
    m_buildLog = new QTextEdit;
    m_buildLog->setReadOnly(true);
    m_buildLog->setFont(QFont("Consolas", 9));
    buildLayout->addWidget(m_buildLog);
    m_buildTime = new QLabel;
    m_buildTime->setStyleSheet(QStringLiteral("color: %1; font-size: 8pt; padding: 2px 4px; background: transparent;").arg(m_colors.textDim));
    buildLayout->addWidget(m_buildTime);
    m_tabs->addTab(buildWidget, "Build");

    // Run tab
    auto *runWidget = new QWidget;
    auto *runLayout = new QVBoxLayout(runWidget);
    runLayout->setContentsMargins(0, 0, 0, 0);
    m_runLog = new QTextEdit;
    m_runLog->setReadOnly(true);
    m_runLog->setFont(QFont("Consolas", 9));
    runLayout->addWidget(m_runLog);
    m_runTime = new QLabel;
    m_runTime->setStyleSheet(QStringLiteral("color: %1; font-size: 8pt; padding: 2px 4px; background: transparent;").arg(m_colors.textDim));
    runLayout->addWidget(m_runTime);
    m_tabs->addTab(runWidget, "Run");

    // Problems tab
    auto *probWidget = new QWidget;
    auto *probLayout = new QVBoxLayout(probWidget);
    probLayout->setContentsMargins(0, 0, 0, 0);
    m_problemsTree = new QTreeWidget;
    m_problemsTree->setHeaderLabels({"File", "Line", "Severity", "Message"});
    m_problemsTree->setRootIsDecorated(false);
    m_problemsTree->setAlternatingRowColors(true);
    auto *ph = m_problemsTree->header();
    ph->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ph->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ph->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ph->setSectionResizeMode(3, QHeaderView::Stretch);
    connect(m_problemsTree, &QTreeWidget::itemDoubleClicked,
            this, &OutputPanel::onProblemDoubleClicked);
    probLayout->addWidget(m_problemsTree);
    m_probTime = new QLabel;
    m_probTime->setStyleSheet(QStringLiteral("color: %1; font-size: 8pt; padding: 2px 4px; background: transparent;").arg(m_colors.textDim));
    probLayout->addWidget(m_probTime);
    m_tabs->addTab(probWidget, "Problems");

    // Profile tab
    auto *profWidget = new QWidget;
    auto *profLayout = new QVBoxLayout(profWidget);
    profLayout->setContentsMargins(0, 0, 0, 0);
    m_profileView = new QTextEdit;
    m_profileView->setReadOnly(true);
    m_profileView->setFont(QFont("Consolas", 9));
    m_profileView->setPlaceholderText("No profile data. Build with profiling to generate ul_profile_summary.json.");
    profLayout->addWidget(m_profileView);
    m_profTime = new QLabel;
    m_profTime->setStyleSheet(QStringLiteral("color: %1; font-size: 8pt; padding: 2px 4px; background: transparent;").arg(m_colors.textDim));
    profLayout->addWidget(m_profTime);
    m_tabs->addTab(profWidget, "Profile");

    // DR Monitor tab
    m_drMonitor = new DRMonitor(m_colors);
    m_tabs->addTab(m_drMonitor, "DR Monitor");

    layout->addWidget(m_tabs);
    setMinimumHeight(30);
    setMaximumHeight(400);
    m_expandedHeight = 220;
}

void OutputPanel::toggleCollapse()
{
    m_collapsed = !m_collapsed;
    m_tabs->setVisible(!m_collapsed);
    if (m_collapsed) {
        m_toggleBtn->setText(QStringLiteral("\u25B6 Output"));  // ▶
        setFixedHeight(30);
    } else {
        m_toggleBtn->setText(QStringLiteral("\u25BC Output"));  // ▼
        setMinimumHeight(30);
        setMaximumHeight(400);
        resize(width(), m_expandedHeight);
    }
}

void OutputPanel::appendLine(QTextEdit *edit, const QString &text,
                              const QString &tag, const ThemeColors &c)
{
    QString color;
    if (tag == "error")        color = c.red;
    else if (tag == "warning") color = c.orange;
    else if (tag == "success") color = c.green;
    else if (tag == "info")    color = c.blue;
    else if (tag == "cmd")     color = c.textDim;
    else                       color = c.text;

    edit->append(QStringLiteral(
        "<span style=\"color:%1; font-family:Consolas;\">%2</span>")
        .arg(color, text.toHtmlEscaped()));

    auto *sb = edit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void OutputPanel::appendBuildLine(const QString &text, const QString &tag)
{
    appendLine(m_buildLog, text, tag, m_colors);
    updateTimestamp(0);
}

void OutputPanel::appendRunLine(const QString &text, const QString &tag)
{
    appendLine(m_runLog, text, tag, m_colors);
    updateTimestamp(1);
}

void OutputPanel::clearBuild()
{
    m_buildLog->clear();
}

void OutputPanel::clearRun()
{
    m_runLog->clear();
}

void OutputPanel::addProblem(const QString &file, int line, int col,
                              const QString &severity, const QString &message)
{
    Q_UNUSED(col)
    auto *item = new QTreeWidgetItem({
        QFileInfo(file).fileName(),
        QString::number(line),
        severity,
        message
    });
    item->setData(0, Qt::UserRole, file);
    item->setData(1, Qt::UserRole, line);

    if (severity == "error")
        for (int i = 0; i < 4; ++i)
            item->setForeground(i, QColor(m_colors.red));
    else if (severity == "warning")
        for (int i = 0; i < 4; ++i)
            item->setForeground(i, QColor(m_colors.orange));

    m_problemsTree->addTopLevelItem(item);
    updateTimestamp(2);
}

void OutputPanel::clearProblems()
{
    m_problemsTree->clear();
}

void OutputPanel::loadProfile(const QString &projectDir)
{
    m_profileView->clear();
    QString path = QDir(projectDir).filePath("ul_profile_summary.json");
    QFile file(path);
    if (!file.exists()) {
        m_profileView->setPlainText("No profile data found.");
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isArray()) {
        QString table = QStringLiteral("%-30s %10s %10s %10s\n").arg("Function", "Calls", "Total ms", "Avg ms");
        table += QString(62, '-') + "\n";
        for (const QJsonValue &v : doc.array()) {
            QJsonObject obj = v.toObject();
            table += QStringLiteral("%-30s %10d %10.2f %10.2f\n")
                .arg(obj["name"].toString())
                .arg(obj["calls"].toInt())
                .arg(obj["total_ms"].toDouble())
                .arg(obj["avg_ms"].toDouble());
        }
        m_profileView->setPlainText(table);
    } else {
        m_profileView->setPlainText(doc.toJson(QJsonDocument::Indented));
    }
    updateTimestamp(3);
}

void OutputPanel::onProblemDoubleClicked(QTreeWidgetItem *item, int)
{
    QString file = item->data(0, Qt::UserRole).toString();
    int line = item->data(1, Qt::UserRole).toInt();
    if (!file.isEmpty() && line > 0)
        emit problemClicked(file, line);
}

void OutputPanel::updateTimestamp(int tabIndex)
{
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    QLabel *labels[] = {m_buildTime, m_runTime, m_probTime, m_profTime};
    if (tabIndex >= 0 && tabIndex < 4)
        labels[tabIndex]->setText(QStringLiteral("Last updated: %1").arg(ts));
}

void OutputPanel::setProjectDir(const QString &dir)
{
    m_drMonitor->setProjectDir(dir);
}

void OutputPanel::onThemeChanged(ThemeMode mode, const ThemeColors &colors)
{
    m_colors = colors;
    m_toggleBtn->setStyleSheet(
        QStringLiteral("QPushButton { background: transparent; border: none;"
                        " color: %1; font-weight: bold; font-size: 9pt; padding: 2px 6px; }"
                        "QPushButton:hover { color: %2; }")
            .arg(m_colors.textDim, m_colors.text));
    m_drMonitor->onThemeChanged(mode, colors);
}
