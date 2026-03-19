// helpdialogs.cpp -- Keyboard shortcuts and About dialog implementations

#include "helpdialogs.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>

// ── ShortcutsDialog ─────────────────────────────────────────────────────

ShortcutsDialog::ShortcutsDialog(const ThemeColors &colors, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Keyboard Shortcuts");
    setMinimumSize(420, 380);
    resize(460, 420);

    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel("Keyboard Shortcuts");
    title->setStyleSheet(QStringLiteral(
        "font-size: 14pt; font-weight: bold; color: %1;").arg(colors.navyLight));
    layout->addWidget(title);

    auto *tree = new QTreeWidget;
    tree->setHeaderLabels({"Shortcut", "Action"});
    tree->setRootIsDecorated(false);
    tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    struct SC { QString key; QString desc; };
    QVector<SC> shortcuts = {
        {"Ctrl+S",         "Save current file"},
        {"Ctrl+B",         "Build"},
        {"Ctrl+R",         "Run"},
        {"Ctrl+W",         "Close current editor tab"},
        {"Ctrl+Tab",       "Next editor tab"},
        {"Ctrl+P",         "Command palette"},
        {"Ctrl+,",         "Open settings"},
        {"Ctrl+Shift+B",   "Build all targets"},
        {"F5",             "Run with profiling"},
        {"F12",            "Go to definition"},
    };

    for (const auto &sc : shortcuts)
        tree->addTopLevelItem(new QTreeWidgetItem({sc.key, sc.desc}));

    layout->addWidget(tree);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
}

// ── AboutDialog ─────────────────────────────────────────────────────────

AboutDialog::AboutDialog(const ThemeColors &colors, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About UniLogic Project Manager");
    setFixedSize(420, 400);

    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(10);

    auto *logo = new QLabel(QStringLiteral("\u221e"));
    logo->setStyleSheet(QStringLiteral(
        "font-size: 56pt; color: %1; background: transparent;").arg(colors.navyLight));
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo);

    auto *title = new QLabel("UniLogic Project Manager");
    title->setStyleSheet(QStringLiteral(
        "font-size: 14pt; font-weight: bold; color: %1; background: transparent;").arg(colors.text));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto *tagline = new QLabel("One language for everything.");
    tagline->setStyleSheet(QStringLiteral(
        "font-size: 10pt; font-style: italic; color: %1; background: transparent;").arg(colors.textDim));
    tagline->setAlignment(Qt::AlignCenter);
    layout->addWidget(tagline);

    auto *version = new QLabel("Version 0.1.0 — XPile 3-17");
    version->setStyleSheet(QStringLiteral(
        "color: %1; background: transparent;").arg(colors.textDim));
    version->setAlignment(Qt::AlignCenter);
    layout->addWidget(version);

    auto *targets = new QLabel("Targets: C, Python, JavaScript, LLVM IR, WebAssembly, Bytecode VM");
    targets->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 9pt; background: transparent;").arg(colors.textDim));
    targets->setAlignment(Qt::AlignCenter);
    targets->setWordWrap(true);
    layout->addWidget(targets);

    auto *builtWith = new QLabel("Built with Qt6");
    builtWith->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 9pt; background: transparent;").arg(colors.textDim));
    builtWith->setAlignment(Qt::AlignCenter);
    layout->addWidget(builtWith);

    auto *website = new QLabel(QStringLiteral(
        "<a href=\"https://albazzaztariq.github.io/UniLogic/\" style=\"color: %1;\">albazzaztariq.github.io/UniLogic</a>").arg(colors.navyLight));
    website->setOpenExternalLinks(true);
    website->setAlignment(Qt::AlignCenter);
    website->setStyleSheet("background: transparent;");
    layout->addWidget(website);

    layout->addSpacing(4);

    auto *copyright = new QLabel(QStringLiteral("\u00A9 2026 UniLogic"));
    copyright->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 9pt; background: transparent;").arg(colors.textDim));
    copyright->setAlignment(Qt::AlignCenter);
    layout->addWidget(copyright);

    layout->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
}
