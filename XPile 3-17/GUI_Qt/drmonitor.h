#pragma once
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFileSystemWatcher>
#include "theme.h"

class DRMonitor : public QWidget
{
    Q_OBJECT
public:
    explicit DRMonitor(const ThemeColors &colors, QWidget *parent = nullptr);
    void setProjectDir(const QString &dir);
public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);
private slots:
    void onFileChanged(const QString &path);
    void clearCounters();
private:
    void parseLine(const QString &line);
    ThemeColors       m_colors;
    QFileSystemWatcher m_watcher;
    QString            m_logPath;
    qint64             m_lastPos = 0;
    QLabel *m_gcLbl = nullptr, *m_rcLbl = nullptr, *m_arenaLbl = nullptr, *m_allocLbl = nullptr;
    int m_gcCount = 0, m_rcCount = 0, m_arenaCount = 0, m_allocCount = 0, m_freeCount = 0;
    qint64 m_liveBytes = 0;
    QTextEdit *m_log = nullptr;
};
