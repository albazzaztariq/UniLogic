#pragma once
// buildtab.h -- "Build" tab with background compilation

#include <QWidget>
#include <QThread>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include "theme.h"

// ── Build worker thread ─────────────────────────────────────────────────

class BuildWorker : public QThread
{
    Q_OBJECT

public:
    BuildWorker(const QString &src, const QString &target,
                const QString &out, const QString &compilerDir,
                QObject *parent = nullptr);

    QString fullCommand() const;

signals:
    void logLine(const QString &text, const QString &tag);
    void finishedBuild(int returnCode);

protected:
    void run() override;

private:
    QString m_src;
    QString m_target;
    QString m_out;
    QString m_compilerDir;
    QString m_fullCommand;
};

// ── Run worker thread ───────────────────────────────────────────────────

class RunWorker : public QThread
{
    Q_OBJECT

public:
    RunWorker(const QString &src, const QString &compilerDir,
              QObject *parent = nullptr);
    QString fullCommand() const { return m_fullCommand; }

signals:
    void logLine(const QString &text, const QString &tag);
    void finishedRun(int returnCode);

protected:
    void run() override;

private:
    QString m_src;
    QString m_compilerDir;
    QString m_fullCommand;
};

// ── Build tab ───────────────────────────────────────────────────────────

class QComboBox;

class BuildTab : public QWidget
{
    Q_OBJECT

public:
    explicit BuildTab(const ThemeColors &colors, QWidget *parent = nullptr);

    void setProjectDir(const QString &dir);

signals:
    void buildLogLine(const QString &text, const QString &tag);
    void runLogLine(const QString &text, const QString &tag);
    void buildStarted();
    void runStarted();
    void buildProblem(const QString &file, int line, int col,
                      const QString &severity, const QString &message);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);
    void build();
    void runProgram();

private slots:
    void browseSource();
    void onLogLine(const QString &text, const QString &tag);
    void onBuildDone(int returnCode);
    void onRunDone(int returnCode);
    void copyLog();
    void clearLog();
    void onProfileSelected(int index);
    void saveProfile();
    void deleteProfile();

private:
    void setActionButtonsEnabled(bool enabled);
    void parseBuildErrors(const QString &text);
    void loadProfiles();
    void writeProfiles();

    ThemeColors   m_colors;
    BuildWorker  *m_worker    = nullptr;
    RunWorker    *m_runWorker = nullptr;
    bool          m_isBuilding = false;
    QString       m_projectDir;

    QLabel       *m_titleLbl      = nullptr;
    QLabel       *m_subtitleLbl   = nullptr;
    QLabel       *m_statusLbl     = nullptr;
    QComboBox    *m_profileCombo  = nullptr;
    QLineEdit    *m_srcEdit       = nullptr;
    QLineEdit    *m_outEdit       = nullptr;
    QPushButton  *m_buildBtn      = nullptr;
    QPushButton  *m_runBtn        = nullptr;
    QPushButton  *m_copyBtn       = nullptr;
    QPushButton  *m_clearBtn      = nullptr;
    QTextEdit    *m_log           = nullptr;
    QButtonGroup *m_targetGroup   = nullptr;
    QString       m_targetValue   = "c";

    struct Profile { QString name, target, drMemory, drSafety, output; int normLevel; };
    QVector<Profile> m_profiles;
};
