// packagestab.cpp -- "Packages" tab implementation

#include "packagestab.h"
#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QProcess>
#include <QDir>
#include <QFont>
#include <QMessageBox>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// ── PackageWorker ───────────────────────────────────────────────────────

PackageWorker::PackageWorker(Action action, const QString &arg,
                             const QString &compilerDir, QObject *parent)
    : QThread(parent), m_action(action), m_arg(arg),
      m_compilerDir(compilerDir)
{
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    QStringList parts = {"python", mainPy};
    switch (m_action) {
    case Search:    parts << "package" << "search" << m_arg; break;
    case Install:   parts << "package" << "install" << m_arg; break;
    case Remove:    parts << "package" << "remove" << m_arg; break;
    case List:      parts << "package" << "list"; break;
    case UpdateAll: parts << "package" << "update"; break;
    }
    m_fullCommand = parts.join(' ');
}

void PackageWorker::run()
{
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    QStringList args = {mainPy};

    switch (m_action) {
    case Search:    args << "package" << "search" << m_arg; break;
    case Install:   args << "package" << "install" << m_arg; break;
    case Remove:    args << "package" << "remove" << m_arg; break;
    case List:      args << "package" << "list"; break;
    case UpdateAll: args << "package" << "update"; break;
    }

    QProcess proc;
    proc.start("python", args);

    if (!proc.waitForStarted(5000)) {
        emit logLine("[error] Could not start python.", "error");
        emit finished(-1);
        return;
    }

    if (!proc.waitForFinished(60000)) {
        emit logLine("[timeout] Operation exceeded 60 seconds.", "error");
        proc.kill();
        emit finished(-1);
        return;
    }

    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    QString err = QString::fromUtf8(proc.readAllStandardError()).trimmed();

    if (!out.isEmpty()) {
        for (const QString &line : out.split('\n'))
            emit logLine(line, "");
    }
    if (!err.isEmpty()) {
        for (const QString &line : err.split('\n'))
            emit logLine(line, "error");
    }

    // Try to parse JSON output for structured results
    if (m_action == Search || m_action == List) {
        QJsonDocument doc = QJsonDocument::fromJson(out.toUtf8());
        if (doc.isArray()) {
            for (const QJsonValue &v : doc.array()) {
                QJsonObject obj = v.toObject();
                QString name = obj["name"].toString();
                QString ver  = obj["version"].toString();
                QString desc = obj["description"].toString();
                if (m_action == Search)
                    emit packageResult(name, ver, desc);
                else
                    emit installedResult(name, ver);
            }
        }
    }

    emit finished(proc.exitCode());
}

// ── PackagesTab ─────────────────────────────────────────────────────────

PackagesTab::PackagesTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    // ── Header ──────────────────────────────────────────────────────────
    m_titleLbl = new QLabel("Packages");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    layout->addWidget(m_titleLbl);

    m_subtitleLbl = new QLabel("Search, install, and manage UniLogic packages.");
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    layout->addWidget(m_subtitleLbl);

    // ── Search row ──────────────────────────────────────────────────────
    auto *searchRow = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search packages...");
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PackagesTab::searchPackages);
    searchRow->addWidget(m_searchEdit, 1);

    m_searchBtn = new QPushButton("Search");
    m_searchBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(m_searchBtn, &QPushButton::clicked, this, &PackagesTab::searchPackages);
    searchRow->addWidget(m_searchBtn);
    layout->addLayout(searchRow);

    // ── Search results ──────────────────────────────────────────────────
    auto *resultsGroup = new QGroupBox("Search Results");
    auto *resultsLayout = new QVBoxLayout(resultsGroup);

    m_resultsTree = new QTreeWidget;
    m_resultsTree->setHeaderLabels({"Package", "Version", "Description", ""});
    m_resultsTree->setRootIsDecorated(false);
    m_resultsTree->setAlternatingRowColors(true);
    auto *rh = m_resultsTree->header();
    rh->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    rh->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    rh->setSectionResizeMode(2, QHeaderView::Stretch);
    rh->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    resultsLayout->addWidget(m_resultsTree);
    layout->addWidget(resultsGroup);

    // ── Installed packages ──────────────────────────────────────────────
    auto *installedGroup = new QGroupBox("Installed Packages");
    auto *installedLayout = new QVBoxLayout(installedGroup);

    auto *installedBtnRow = new QHBoxLayout;
    installedBtnRow->addStretch();

    m_refreshBtn = new QPushButton("Refresh");
    connect(m_refreshBtn, &QPushButton::clicked, this, &PackagesTab::refreshInstalled);
    installedBtnRow->addWidget(m_refreshBtn);

    m_updateBtn = new QPushButton("Update All");
    m_updateBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(m_updateBtn, &QPushButton::clicked, this, &PackagesTab::updateAll);
    installedBtnRow->addWidget(m_updateBtn);
    installedLayout->addLayout(installedBtnRow);

    m_installedTree = new QTreeWidget;
    m_installedTree->setHeaderLabels({"Package", "Version", ""});
    m_installedTree->setRootIsDecorated(false);
    m_installedTree->setAlternatingRowColors(true);
    auto *ih = m_installedTree->header();
    ih->setSectionResizeMode(0, QHeaderView::Stretch);
    ih->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ih->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    installedLayout->addWidget(m_installedTree);
    layout->addWidget(installedGroup);

    // ── Log panel ───────────────────────────────────────────────────────
    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setFont(QFont("Consolas", 9));
    m_log->setMaximumHeight(100);
    m_log->setPlaceholderText("Package operation output...");
    layout->addWidget(m_log);
}

void PackagesTab::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    m_searchBtn->setStyleSheet(navyBtnStyle(m_colors));
    m_updateBtn->setStyleSheet(navyBtnStyle(m_colors));
}

void PackagesTab::setButtonsEnabled(bool enabled)
{
    m_searchBtn->setEnabled(enabled);
    m_updateBtn->setEnabled(enabled);
    m_refreshBtn->setEnabled(enabled);
}

void PackagesTab::runWorker(PackageWorker::Action action, const QString &arg)
{
    QString compilerDir = SettingsDialog::loadCompilerDir();
    if (compilerDir.isEmpty()) {
        QMessageBox::warning(this, "No Compiler Path",
            "Set the compiler path in Settings first.");
        return;
    }

    setButtonsEnabled(false);
    m_log->clear();

    m_worker = new PackageWorker(action, arg, compilerDir, this);

    onLogLine(QStringLiteral("> %1").arg(m_worker->fullCommand()), "cmd");

    connect(m_worker, &PackageWorker::logLine, this, &PackagesTab::onLogLine);
    connect(m_worker, &PackageWorker::finished, this, &PackagesTab::onWorkerDone);
    connect(m_worker, &PackageWorker::packageResult, this, &PackagesTab::onPackageResult);
    connect(m_worker, &PackageWorker::installedResult, this, &PackagesTab::onInstalledResult);
    connect(m_worker, &QThread::finished, m_worker, &QObject::deleteLater);
    m_worker->start();
}

void PackagesTab::searchPackages()
{
    QString query = m_searchEdit->text().trimmed();
    if (query.isEmpty()) return;
    m_resultsTree->clear();
    runWorker(PackageWorker::Search, query);
}

void PackagesTab::installPackage()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString pkg = btn->property("packageName").toString();
    if (!pkg.isEmpty())
        runWorker(PackageWorker::Install, pkg);
}

void PackagesTab::removePackage()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString pkg = btn->property("packageName").toString();
    if (!pkg.isEmpty())
        runWorker(PackageWorker::Remove, pkg);
}

void PackagesTab::updateAll()
{
    runWorker(PackageWorker::UpdateAll, QString());
}

void PackagesTab::refreshInstalled()
{
    m_installedTree->clear();
    runWorker(PackageWorker::List, QString());
}

void PackagesTab::onLogLine(const QString &text, const QString &tag)
{
    QString color;
    if (tag == "error")     color = m_colors.red;
    else if (tag == "cmd")  color = m_colors.textDim;
    else                    color = m_colors.text;

    m_log->append(QStringLiteral(
        "<span style=\"color:%1; font-family:Consolas;\">%2</span>")
        .arg(color, text.toHtmlEscaped()));

    auto *sb = m_log->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void PackagesTab::onWorkerDone(int)
{
    setButtonsEnabled(true);
}

void PackagesTab::onPackageResult(const QString &name, const QString &version,
                                   const QString &desc)
{
    auto *item = new QTreeWidgetItem({name, version, desc, ""});
    m_resultsTree->addTopLevelItem(item);

    auto *installBtn = new QPushButton("Install");
    installBtn->setStyleSheet(navyBtnStyle(m_colors));
    installBtn->setProperty("packageName", name);
    connect(installBtn, &QPushButton::clicked, this, &PackagesTab::installPackage);
    m_resultsTree->setItemWidget(item, 3, installBtn);
}

void PackagesTab::onInstalledResult(const QString &name, const QString &version)
{
    auto *item = new QTreeWidgetItem({name, version, ""});
    m_installedTree->addTopLevelItem(item);

    auto *removeBtn = new QPushButton("Remove");
    removeBtn->setProperty("packageName", name);
    connect(removeBtn, &QPushButton::clicked, this, &PackagesTab::removePackage);
    m_installedTree->setItemWidget(item, 2, removeBtn);
}
