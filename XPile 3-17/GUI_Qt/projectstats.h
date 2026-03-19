#pragma once
#include <QDialog>
#include "theme.h"

class ProjectStatsDialog : public QDialog
{
    Q_OBJECT
public:
    ProjectStatsDialog(const QString &projectDir, const ThemeColors &colors,
                       QWidget *parent = nullptr);
};
