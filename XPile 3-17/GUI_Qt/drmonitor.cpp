#include "drmonitor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>
#include <QFont>
#include <QDir>

DRMonitor::DRMonitor(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *counterRow = new QHBoxLayout;
    auto makeCounter = [&](const QString &label, const QString &color) -> QLabel* {
        auto *lbl = new QLabel(QStringLiteral("%1: 0").arg(label));
        lbl->setStyleSheet(QStringLiteral(
            "color: %1; font-weight: bold; font-family: Consolas; font-size: 10pt; background: transparent; padding: 2px 8px;").arg(color));
        counterRow->addWidget(lbl);
        return lbl;
    };
    m_gcLbl    = makeCounter("GC",     m_colors.green);
    m_rcLbl    = makeCounter("RefCnt", m_colors.blue);
    m_arenaLbl = makeCounter("Arena",  m_colors.orange);
    m_allocLbl = makeCounter("Alloc",  m_colors.text);
    counterRow->addStretch();

    auto *clearBtn = new QPushButton("Clear");
    clearBtn->setToolTip("Reset counters and log");
    connect(clearBtn, &QPushButton::clicked, this, &DRMonitor::clearCounters);
    counterRow->addWidget(clearBtn);
    layout->addLayout(counterRow);

    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setFont(QFont("Consolas", 9));
    m_log->setPlaceholderText("Waiting for DR profile events (ul_profile.log)...");
    layout->addWidget(m_log);

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &DRMonitor::onFileChanged);
}

void DRMonitor::setProjectDir(const QString &dir)
{
    if (!m_logPath.isEmpty())
        m_watcher.removePath(m_logPath);
    m_logPath = QDir(dir).filePath("ul_profile.log");
    m_lastPos = 0;
    if (QFile::exists(m_logPath))
        m_watcher.addPath(m_logPath);
    // Also watch the directory for file creation
    m_watcher.addPath(dir);
}

void DRMonitor::onFileChanged(const QString &path)
{
    // If directory changed, check if log appeared
    if (QFileInfo(path).isDir()) {
        if (QFile::exists(m_logPath) && !m_watcher.files().contains(m_logPath))
            m_watcher.addPath(m_logPath);
        return;
    }
    QFile file(m_logPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    file.seek(m_lastPos);
    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        if (!line.isEmpty()) parseLine(line);
    }
    m_lastPos = file.pos();
    file.close();
    // Re-add path (QFileSystemWatcher removes after signal on some platforms)
    if (!m_watcher.files().contains(m_logPath))
        m_watcher.addPath(m_logPath);
}

void DRMonitor::parseLine(const QString &line)
{
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());
    if (!doc.isObject()) {
        m_log->append(QStringLiteral("<span style=\"color:%1;\">%2</span>")
            .arg(m_colors.text, line.toHtmlEscaped()));
        return;
    }
    QJsonObject obj = doc.object();
    QString event = obj["event"].toString();
    QString color = m_colors.text;

    if (event == "gc_collect")     { m_gcCount++;    color = m_colors.green; }
    else if (event == "rc_inc" || event == "rc_dec") { m_rcCount++; color = m_colors.blue; }
    else if (event == "arena_reset") { m_arenaCount++; color = m_colors.orange; }
    else if (event == "alloc")     { m_allocCount++; m_liveBytes += obj["bytes"].toInteger(); }
    else if (event == "free")      { m_freeCount++;  m_liveBytes -= obj["bytes"].toInteger(); }

    m_gcLbl->setText(QStringLiteral("GC: %1").arg(m_gcCount));
    m_rcLbl->setText(QStringLiteral("RefCnt: %1").arg(m_rcCount));
    m_arenaLbl->setText(QStringLiteral("Arena: %1").arg(m_arenaCount));
    m_allocLbl->setText(QStringLiteral("Alloc: %1 / Free: %2 / Live: %3 B")
        .arg(m_allocCount).arg(m_freeCount).arg(m_liveBytes));

    m_log->append(QStringLiteral("<span style=\"color:%1; font-family:Consolas;\">[%2] %3</span>")
        .arg(color, event, line.toHtmlEscaped()));
    auto *sb = m_log->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void DRMonitor::clearCounters()
{
    m_gcCount = m_rcCount = m_arenaCount = m_allocCount = m_freeCount = 0;
    m_liveBytes = 0;
    m_gcLbl->setText("GC: 0");
    m_rcLbl->setText("RefCnt: 0");
    m_arenaLbl->setText("Arena: 0");
    m_allocLbl->setText("Alloc: 0 / Free: 0 / Live: 0 B");
    m_log->clear();
}

void DRMonitor::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    m_gcLbl->setStyleSheet(QStringLiteral("color: %1; font-weight: bold; font-family: Consolas; font-size: 10pt; background: transparent; padding: 2px 8px;").arg(m_colors.green));
    m_rcLbl->setStyleSheet(QStringLiteral("color: %1; font-weight: bold; font-family: Consolas; font-size: 10pt; background: transparent; padding: 2px 8px;").arg(m_colors.blue));
    m_arenaLbl->setStyleSheet(QStringLiteral("color: %1; font-weight: bold; font-family: Consolas; font-size: 10pt; background: transparent; padding: 2px 8px;").arg(m_colors.orange));
}
