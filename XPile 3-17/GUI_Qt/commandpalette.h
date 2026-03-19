#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include "theme.h"

struct PaletteCommand {
    QString name;
    QString shortcut;
    std::function<void()> action;
};

class CommandPalette : public QDialog
{
    Q_OBJECT
public:
    explicit CommandPalette(const QVector<PaletteCommand> &commands,
                            const ThemeColors &colors, QWidget *parent = nullptr);
private slots:
    void filterCommands(const QString &text);
    void executeSelected();
private:
    QVector<PaletteCommand> m_commands;
    QLineEdit   *m_input = nullptr;
    QListWidget *m_list  = nullptr;
};
