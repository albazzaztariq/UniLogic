#pragma once
#include <QDialog>
#include <QPlainTextEdit>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include "theme.h"

class NormDiffViewer : public QDialog
{
    Q_OBJECT
public:
    NormDiffViewer(const QString &filePath, const QString &compilerDir,
                   const ThemeColors &colors, QWidget *parent = nullptr);
private slots:
    void onSliderChanged(int value);
    void runNormalization();
private:
    ThemeColors    m_colors;
    QString        m_filePath;
    QString        m_compilerDir;
    QString        m_originalText;
    QPlainTextEdit *m_leftPanel  = nullptr;
    QPlainTextEdit *m_rightPanel = nullptr;
    QSlider        *m_slider     = nullptr;
    QLabel         *m_levelLbl   = nullptr;
    QTimer          m_debounce;
};
