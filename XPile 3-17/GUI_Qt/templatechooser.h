#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include "theme.h"

class TemplateChooser : public QDialog
{
    Q_OBJECT
public:
    explicit TemplateChooser(const ThemeColors &colors, QWidget *parent = nullptr);
    QString selectedContent() const { return m_content; }
    QString selectedName() const { return m_name; }
private slots:
    void onSelectionChanged();
private:
    struct Template { QString name; QString content; };
    QVector<Template> m_templates;
    QListWidget    *m_list    = nullptr;
    QPlainTextEdit *m_preview = nullptr;
    QString         m_content;
    QString         m_name;
};
