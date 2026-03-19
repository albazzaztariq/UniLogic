#pragma once
// drconflicttab.h -- "DR Conflicts" tab

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QLabel>
#include "theme.h"

class DRConflictTab : public QWidget
{
    Q_OBJECT

public:
    explicit DRConflictTab(const ThemeColors &colors, QWidget *parent = nullptr);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

private slots:
    void browse();
    void scan();

private:
    ThemeColors m_colors;

    QLabel       *m_titleLbl    = nullptr;
    QLabel       *m_subtitleLbl = nullptr;
    QLabel       *m_summaryLbl  = nullptr;
    QLineEdit    *m_folderEdit  = nullptr;
    QTreeWidget  *m_tree        = nullptr;
};
