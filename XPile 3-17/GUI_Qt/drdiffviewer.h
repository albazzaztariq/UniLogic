#pragma once
// drdiffviewer.h -- DR Diff viewer dialog

#include <QDialog>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include "theme.h"

class DRDiffViewer : public QDialog
{
    Q_OBJECT
public:
    explicit DRDiffViewer(const QString &projectDir,
                          const ThemeColors &colors,
                          QWidget *parent = nullptr);

signals:
    void cellClicked(const QString &filePath);

private slots:
    void exportCsv();
    void onItemClicked(QTreeWidgetItem *item, int column);

private:
    ThemeColors  m_colors;
    QString      m_projectDir;
    QTreeWidget *m_tree = nullptr;
};
