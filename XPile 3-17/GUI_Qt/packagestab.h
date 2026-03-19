#pragma once
// packagestab.h -- "Packages" tab with search, install, remove

#include <QWidget>
#include <QThread>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QLabel>
#include "theme.h"

// ── Package worker thread ───────────────────────────────────────────────

class PackageWorker : public QThread
{
    Q_OBJECT

public:
    enum Action { Search, Install, Remove, List, UpdateAll };

    PackageWorker(Action action, const QString &arg,
                  const QString &compilerDir, QObject *parent = nullptr);

    QString fullCommand() const { return m_fullCommand; }

signals:
    void logLine(const QString &text, const QString &tag);
    void finished(int returnCode);
    void packageResult(const QString &name, const QString &version,
                       const QString &description);
    void installedResult(const QString &name, const QString &version);

protected:
    void run() override;

private:
    Action  m_action;
    QString m_arg;
    QString m_compilerDir;
    QString m_fullCommand;
};

// ── Packages tab ────────────────────────────────────────────────────────

class PackagesTab : public QWidget
{
    Q_OBJECT

public:
    explicit PackagesTab(const ThemeColors &colors, QWidget *parent = nullptr);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

private slots:
    void searchPackages();
    void installPackage();
    void removePackage();
    void updateAll();
    void refreshInstalled();
    void onLogLine(const QString &text, const QString &tag);
    void onWorkerDone(int returnCode);
    void onPackageResult(const QString &name, const QString &version,
                         const QString &desc);
    void onInstalledResult(const QString &name, const QString &version);

private:
    void runWorker(PackageWorker::Action action, const QString &arg);
    void setButtonsEnabled(bool enabled);

    ThemeColors    m_colors;
    PackageWorker *m_worker = nullptr;

    QLabel       *m_titleLbl     = nullptr;
    QLabel       *m_subtitleLbl  = nullptr;
    QLineEdit    *m_searchEdit   = nullptr;
    QPushButton  *m_searchBtn    = nullptr;
    QTreeWidget  *m_resultsTree  = nullptr;
    QTreeWidget  *m_installedTree = nullptr;
    QPushButton  *m_updateBtn    = nullptr;
    QPushButton  *m_refreshBtn   = nullptr;
    QTextEdit    *m_log          = nullptr;
};
