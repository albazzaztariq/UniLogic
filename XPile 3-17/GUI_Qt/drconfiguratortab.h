#pragma once
// drconfiguratortab.h -- "DR Config" tab with live conflict checking

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QMap>
#include "theme.h"

class DRConfiguratorTab : public QWidget
{
    Q_OBJECT

public:
    explicit DRConfiguratorTab(const ThemeColors &colors, QWidget *parent = nullptr);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

private slots:
    void browseFile();
    void loadFile();
    void updatePreview();
    void writeToFile();
    void applyPreset(const QString &presetName);
    void resetDefaults();
    void checkConflicts();

private:
    QStringList getDrLines() const;

    ThemeColors m_colors;

    QLabel         *m_titleLbl    = nullptr;
    QLabel         *m_subtitleLbl = nullptr;
    QLabel         *m_writeStatus = nullptr;
    QLabel         *m_conflictLbl = nullptr;
    QLineEdit      *m_fileEdit    = nullptr;
    QPlainTextEdit *m_preview     = nullptr;

    QMap<QString, QCheckBox*>  m_enableChecks;
    QMap<QString, QComboBox*>  m_settingCombos;
};
