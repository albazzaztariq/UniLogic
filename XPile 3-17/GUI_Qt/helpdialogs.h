#pragma once
// helpdialogs.h -- Keyboard shortcuts and About dialogs

#include <QDialog>
#include "theme.h"

class ShortcutsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ShortcutsDialog(const ThemeColors &colors, QWidget *parent = nullptr);
};

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(const ThemeColors &colors, QWidget *parent = nullptr);
};
