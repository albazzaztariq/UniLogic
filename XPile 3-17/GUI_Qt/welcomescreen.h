#pragma once
// welcomescreen.h -- Welcome screen shown on first launch / no project

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include "theme.h"

class WelcomeScreen : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeScreen(const ThemeColors &colors, QWidget *parent = nullptr);

    void setRecentProjects(const QStringList &projects);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

signals:
    void newProjectClicked();
    void openProjectClicked();
    void quickSetupClicked();
    void recentProjectClicked(const QString &path);

private slots:
    void onRecentItemClicked(QListWidgetItem *item);

private:
    ThemeColors   m_colors;
    QLabel       *m_logoLbl     = nullptr;
    QLabel       *m_titleLbl    = nullptr;
    QPushButton  *m_newBtn      = nullptr;
    QPushButton  *m_openBtn     = nullptr;
    QPushButton  *m_setupBtn    = nullptr;
    QLabel       *m_recentLbl   = nullptr;
    QListWidget  *m_recentList  = nullptr;
};
