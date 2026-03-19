#pragma once
// outputpanel.h -- Tabbed output panel (Build, Run, Problems, Profile)

#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include "theme.h"

class DRMonitor;

class OutputPanel : public QWidget
{
    Q_OBJECT
public:
    explicit OutputPanel(const ThemeColors &colors, QWidget *parent = nullptr);

    void appendBuildLine(const QString &text, const QString &tag);
    void appendRunLine(const QString &text, const QString &tag);
    void clearBuild();
    void clearRun();
    void addProblem(const QString &file, int line, int col,
                    const QString &severity, const QString &message);
    void clearProblems();
    void loadProfile(const QString &projectDir);
    void setProjectDir(const QString &dir);
    DRMonitor *drMonitor() const { return m_drMonitor; }

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

signals:
    void problemClicked(const QString &file, int line);

private slots:
    void toggleCollapse();
    void onProblemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void appendLine(QTextEdit *edit, const QString &text,
                    const QString &tag, const ThemeColors &c);
    void updateTimestamp(int tabIndex);

    ThemeColors   m_colors;
    bool          m_collapsed = false;
    int           m_expandedHeight = 220;

    QPushButton  *m_toggleBtn  = nullptr;
    QTabWidget   *m_tabs       = nullptr;
    QTextEdit    *m_buildLog   = nullptr;
    QTextEdit    *m_runLog     = nullptr;
    QTreeWidget  *m_problemsTree = nullptr;
    QTextEdit    *m_profileView  = nullptr;
    QLabel       *m_buildTime  = nullptr;
    QLabel       *m_runTime    = nullptr;
    QLabel       *m_probTime   = nullptr;
    QLabel       *m_profTime   = nullptr;
    DRMonitor    *m_drMonitor  = nullptr;
};
