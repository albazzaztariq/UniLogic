#pragma once
// setupwizard.h -- First-run setup wizard

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include "theme.h"

class SetupWizard : public QDialog
{
    Q_OBJECT
public:
    explicit SetupWizard(const ThemeColors &colors, QWidget *parent = nullptr);

    QString pythonPath() const;
    QString compilerDir() const;

private slots:
    void nextStep();
    void prevStep();
    void browsePython();
    void browseCompiler();
    void testPython();
    void testCompiler();

private:
    void updateButtons();

    ThemeColors     m_colors;
    QStackedWidget *m_pages       = nullptr;
    QPushButton    *m_backBtn     = nullptr;
    QPushButton    *m_nextBtn     = nullptr;

    // Page 0: Python
    QLineEdit      *m_pythonEdit  = nullptr;
    QLabel         *m_pythonStatus = nullptr;

    // Page 1: Compiler
    QLineEdit      *m_compEdit    = nullptr;
    QLabel         *m_compStatus  = nullptr;

    // Page 2: Done
    QLabel         *m_doneLbl     = nullptr;
};
