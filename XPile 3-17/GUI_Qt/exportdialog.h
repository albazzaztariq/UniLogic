#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include "theme.h"

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    ExportDialog(const QString &projectDir, const QString &compilerDir,
                 const ThemeColors &colors, QWidget *parent = nullptr);
private slots:
    void browseOutput();
    void doExport();
private:
    ThemeColors m_colors;
    QString     m_projectDir;
    QString     m_compilerDir;
    QComboBox  *m_formatCombo = nullptr;
    QLineEdit  *m_outputEdit  = nullptr;
    QTextEdit  *m_log         = nullptr;
    QPushButton *m_exportBtn  = nullptr;
};
