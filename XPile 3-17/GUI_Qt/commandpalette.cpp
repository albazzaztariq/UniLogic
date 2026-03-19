#include "commandpalette.h"
#include <QVBoxLayout>
#include <QKeyEvent>

CommandPalette::CommandPalette(const QVector<PaletteCommand> &commands,
                               const ThemeColors &colors, QWidget *parent)
    : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint), m_commands(commands)
{
    setFixedWidth(500);
    setStyleSheet(QStringLiteral(
        "QDialog { background: %1; border: 2px solid %2; border-radius: 10px; }")
        .arg(colors.bgSurface, colors.navyLight));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    m_input = new QLineEdit;
    m_input->setPlaceholderText("Type a command...");
    m_input->setFont(QFont("Segoe UI", 12));
    layout->addWidget(m_input);

    m_list = new QListWidget;
    m_list->setMaximumHeight(300);
    m_list->setStyleSheet(QStringLiteral(
        "QListWidget { background: %1; border: none; color: %2; }"
        "QListWidget::item { padding: 6px 8px; }"
        "QListWidget::item:selected { background: %3; color: white; }")
        .arg(colors.bgSurface, colors.text, colors.navy));
    layout->addWidget(m_list);

    connect(m_input, &QLineEdit::textChanged, this, &CommandPalette::filterCommands);
    connect(m_input, &QLineEdit::returnPressed, this, &CommandPalette::executeSelected);
    connect(m_list, &QListWidget::itemActivated, this, [this](QListWidgetItem*) { executeSelected(); });

    // Populate
    filterCommands("");
    m_input->setFocus();
}

void CommandPalette::filterCommands(const QString &text)
{
    m_list->clear();
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto &cmd = m_commands[i];
        if (text.isEmpty() || cmd.name.contains(text, Qt::CaseInsensitive)) {
            QString label = cmd.name;
            if (!cmd.shortcut.isEmpty())
                label += QStringLiteral("    [%1]").arg(cmd.shortcut);
            auto *item = new QListWidgetItem(label);
            item->setData(Qt::UserRole, i);
            m_list->addItem(item);
        }
    }
    if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

void CommandPalette::executeSelected()
{
    auto *item = m_list->currentItem();
    if (!item) { close(); return; }
    int idx = item->data(Qt::UserRole).toInt();
    close();
    if (idx >= 0 && idx < m_commands.size() && m_commands[idx].action)
        m_commands[idx].action();
}
