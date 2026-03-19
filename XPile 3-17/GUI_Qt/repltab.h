#pragma once
#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QProcess>
#include <QStringList>
#include "theme.h"

class ReplTab : public QWidget
{
    Q_OBJECT
public:
    explicit ReplTab(const ThemeColors &colors, QWidget *parent = nullptr);
    ~ReplTab();

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

private slots:
    void executeInput();
    void onProcessOutput();
    void onProcessError();
    void clearOutput();
    void historyUp();
    void historyDown();

private:
    void appendOutput(const QString &text, const QString &color);
    void startProcess();

    ThemeColors    m_colors;
    QPlainTextEdit *m_output    = nullptr;
    QLineEdit      *m_input     = nullptr;
    QPushButton    *m_runBtn    = nullptr;
    QPushButton    *m_clearBtn  = nullptr;
    QProcess       *m_process   = nullptr;
    QStringList     m_history;
    int             m_historyIdx = -1;
};
