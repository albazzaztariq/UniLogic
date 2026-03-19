#pragma once
// filetreepanel.h -- Collapsible project file tree panel

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QLabel>
#include "theme.h"

class FileTreePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreePanel(const ThemeColors &colors, QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

signals:
    void fileActivated(const QString &filePath);

private slots:
    void onItemDoubleClicked(const QModelIndex &index);
    void onContextMenu(const QPoint &pos);
    void newFile();
    void deleteFile();
    void renameFile();

private:
    ThemeColors       m_colors;
    QFileSystemModel *m_model    = nullptr;
    QTreeView        *m_tree     = nullptr;
    QLabel           *m_titleLbl = nullptr;
    QPushButton      *m_collapseBtn = nullptr;
    QString           m_rootPath;
};
