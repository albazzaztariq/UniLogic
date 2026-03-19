#pragma once
// newprojecttab.h -- "New Project" tab

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include "theme.h"

class NewProjectTab : public QWidget
{
    Q_OBJECT

public:
    explicit NewProjectTab(const ThemeColors &colors, QWidget *parent = nullptr);

signals:
    void projectCreated(const QString &projectDir);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

private slots:
    void browse();
    void createProject();

private:
    QString makeEntryContent(const QString &name) const;

    ThemeColors m_colors;

    QLabel      *m_titleLbl   = nullptr;
    QLabel      *m_subtitleLbl = nullptr;
    QLabel      *m_statusLbl  = nullptr;
    QLineEdit   *m_nameEdit   = nullptr;
    QLineEdit   *m_locEdit    = nullptr;
    QLineEdit   *m_entryEdit  = nullptr;
    QComboBox   *m_presetCombo = nullptr;
    QPushButton *m_createBtn  = nullptr;
    QMap<QString, QCheckBox*> m_dirChecks;
};
