// welcomescreen.cpp -- Welcome screen implementation

#include "welcomescreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

WelcomeScreen::WelcomeScreen(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->addStretch(2);

    // ── Center column ───────────────────────────────────────────────────
    auto *centerLayout = new QVBoxLayout;
    centerLayout->setAlignment(Qt::AlignHCenter);
    centerLayout->setSpacing(12);

    // Logo (infinity symbol, large)
    m_logoLbl = new QLabel(QStringLiteral("\u221e"));
    m_logoLbl->setStyleSheet(
        QStringLiteral("font-size: 72pt; color: %1; background: transparent;")
            .arg(m_colors.navyLight));
    m_logoLbl->setAlignment(Qt::AlignCenter);
    centerLayout->addWidget(m_logoLbl);

    // Title
    m_titleLbl = new QLabel("UniLogic Project Manager");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 18pt; font-weight: bold; color: %1; background: transparent;")
            .arg(m_colors.text));
    m_titleLbl->setAlignment(Qt::AlignCenter);
    centerLayout->addWidget(m_titleLbl);

    centerLayout->addSpacing(24);

    // ── Action buttons ──────────────────────────────────────────────────
    auto *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(16);
    btnLayout->addStretch();

    auto makeBtn = [&](const QString &label, const QString &icon) {
        auto *btn = new QPushButton(icon + "  " + label);
        btn->setFixedSize(200, 52);
        btn->setStyleSheet(
            QStringLiteral(
                "QPushButton { background-color: %1; color: white; font-weight: bold;"
                " font-size: 11pt; border: none; border-radius: 8px; }"
                "QPushButton:hover { background-color: %2; }")
                .arg(m_colors.navy, m_colors.navyHover));
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    m_newBtn   = makeBtn("New Project",  QStringLiteral("\u2795"));
    m_openBtn  = makeBtn("Open Project", QStringLiteral("\U0001F4C2"));
    m_setupBtn = makeBtn("Quick Setup",  QStringLiteral("\u2699"));

    connect(m_newBtn,   &QPushButton::clicked, this, &WelcomeScreen::newProjectClicked);
    connect(m_openBtn,  &QPushButton::clicked, this, &WelcomeScreen::openProjectClicked);
    connect(m_setupBtn, &QPushButton::clicked, this, &WelcomeScreen::quickSetupClicked);

    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_openBtn);
    btnLayout->addWidget(m_setupBtn);
    btnLayout->addStretch();
    centerLayout->addLayout(btnLayout);

    centerLayout->addSpacing(30);

    // ── Recent projects ─────────────────────────────────────────────────
    m_recentLbl = new QLabel("Recent Projects");
    m_recentLbl->setStyleSheet(
        QStringLiteral("font-size: 11pt; font-weight: bold; color: %1; background: transparent;")
            .arg(m_colors.textDim));
    m_recentLbl->setAlignment(Qt::AlignCenter);
    centerLayout->addWidget(m_recentLbl);

    m_recentList = new QListWidget;
    m_recentList->setFixedWidth(500);
    m_recentList->setMaximumHeight(160);
    m_recentList->setStyleSheet(
        QStringLiteral(
            "QListWidget { background: %1; border: 1px solid %2; border-radius: 6px;"
            " color: %3; font-size: 10pt; }"
            "QListWidget::item { padding: 6px 10px; }"
            "QListWidget::item:hover { background: %4; }"
            "QListWidget::item:selected { background: %5; color: white; }")
            .arg(m_colors.bgSurface, m_colors.border, m_colors.text,
                 m_colors.bgRaised, m_colors.navy));
    connect(m_recentList, &QListWidget::itemClicked,
            this, &WelcomeScreen::onRecentItemClicked);

    auto *listCenter = new QHBoxLayout;
    listCenter->addStretch();
    listCenter->addWidget(m_recentList);
    listCenter->addStretch();
    centerLayout->addLayout(listCenter);

    outerLayout->addLayout(centerLayout);
    outerLayout->addStretch(3);
}

void WelcomeScreen::setRecentProjects(const QStringList &projects)
{
    m_recentList->clear();
    if (projects.isEmpty()) {
        m_recentLbl->setText("No Recent Projects");
        m_recentList->setVisible(false);
        return;
    }
    m_recentLbl->setText("Recent Projects");
    m_recentList->setVisible(true);
    for (const QString &path : projects) {
        auto *item = new QListWidgetItem(path);
        item->setData(Qt::UserRole, path);
        m_recentList->addItem(item);
    }
}

void WelcomeScreen::onRecentItemClicked(QListWidgetItem *item)
{
    QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty())
        emit recentProjectClicked(path);
}

void WelcomeScreen::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    m_logoLbl->setStyleSheet(
        QStringLiteral("font-size: 72pt; color: %1; background: transparent;")
            .arg(m_colors.navyLight));
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 18pt; font-weight: bold; color: %1; background: transparent;")
            .arg(m_colors.text));
    m_recentLbl->setStyleSheet(
        QStringLiteral("font-size: 11pt; font-weight: bold; color: %1; background: transparent;")
            .arg(m_colors.textDim));
}
