// buildtab.cpp -- "Build" tab implementation with background compilation

#include "buildtab.h"
#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QClipboard>
#include <QApplication>
#include <QScrollBar>
#include <QRegularExpression>
#include <QComboBox>
#include <QInputDialog>
#include <QTextStream>

// ── BuildWorker ─────────────────────────────────────────────────────────

BuildWorker::BuildWorker(const QString &src, const QString &target,
                         const QString &out, const QString &compilerDir,
                         QObject *parent)
    : QThread(parent), m_src(src), m_target(target), m_out(out),
      m_compilerDir(compilerDir)
{
    // Build the full command string for display
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    QStringList parts;
    parts << "python" << mainPy << m_src << "-t" << m_target;
    if (!m_out.isEmpty() && m_out != "(auto)")
        parts << "-o" << m_out;
    m_fullCommand = parts.join(' ');
}

QString BuildWorker::fullCommand() const
{
    return m_fullCommand;
}

void BuildWorker::run()
{
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");

    QStringList args;
    args << mainPy << m_src << "-t" << m_target;
    if (!m_out.isEmpty() && m_out != "(auto)")
        args << "-o" << m_out;

    QProcess proc;
    proc.setWorkingDirectory(QFileInfo(m_src).absolutePath());
    proc.start("python", args);

    if (!proc.waitForStarted(5000)) {
        emit logLine("[error] Could not start python. Is it on PATH?", "error");
        emit finishedBuild(-1);
        return;
    }

    if (!proc.waitForFinished(60000)) {
        emit logLine("[timeout] Build exceeded 60 seconds.", "error");
        proc.kill();
        emit finishedBuild(-1);
        return;
    }

    QString stdoutStr = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    QString stderrStr = QString::fromUtf8(proc.readAllStandardError()).trimmed();

    if (!stdoutStr.isEmpty()) {
        for (const QString &line : stdoutStr.split('\n')) {
            QString lower = line.toLower();
            QString tag;
            if (lower.contains("compiled") || lower.contains("success"))
                tag = "success";
            else if (lower.contains("warning"))
                tag = "warning";
            emit logLine(line, tag);
        }
    }
    if (!stderrStr.isEmpty()) {
        for (const QString &line : stderrStr.split('\n')) {
            QString lower = line.toLower();
            QString tag;
            if (lower.contains("warning"))
                tag = "warning";
            else
                tag = "error";
            emit logLine(line, tag);
        }
    }

    emit finishedBuild(proc.exitCode());
}

// ── RunWorker ───────────────────────────────────────────────────────────

RunWorker::RunWorker(const QString &src, const QString &compilerDir,
                     QObject *parent)
    : QThread(parent), m_src(src), m_compilerDir(compilerDir)
{
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    m_fullCommand = QStringLiteral("python %1 run %2").arg(mainPy, m_src);
}

void RunWorker::run()
{
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    QStringList args = {mainPy, "run", m_src};

    QProcess proc;
    proc.setWorkingDirectory(QFileInfo(m_src).absolutePath());
    proc.start("python", args);

    if (!proc.waitForStarted(5000)) {
        emit logLine("[error] Could not start python.", "error");
        emit finishedRun(-1);
        return;
    }

    if (!proc.waitForFinished(60000)) {
        emit logLine("[timeout] Run exceeded 60 seconds.", "error");
        proc.kill();
        emit finishedRun(-1);
        return;
    }

    QString stdoutStr = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    QString stderrStr = QString::fromUtf8(proc.readAllStandardError()).trimmed();

    if (!stdoutStr.isEmpty()) {
        for (const QString &line : stdoutStr.split('\n'))
            emit logLine(line, "");
    }
    if (!stderrStr.isEmpty()) {
        for (const QString &line : stderrStr.split('\n'))
            emit logLine(line, "error");
    }

    emit finishedRun(proc.exitCode());
}

// ── BuildTab ────────────────────────────────────────────────────────────

BuildTab::BuildTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    // ── Header ──────────────────────────────────────────────────────────
    m_titleLbl = new QLabel("Build");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    layout->addWidget(m_titleLbl);

    m_subtitleLbl = new QLabel("Compile a .ul file using the XPile pipeline.");
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    layout->addWidget(m_subtitleLbl);

    // ── Build config ────────────────────────────────────────────────────
    auto *cfgGroup  = new QGroupBox("Build Configuration");
    auto *cfgLayout = new QGridLayout(cfgGroup);
    cfgLayout->setColumnStretch(1, 1);
    cfgLayout->setVerticalSpacing(10);
    cfgLayout->setHorizontalSpacing(12);

    // Row 0: Profile
    auto *profLbl = new QLabel("Profile:");
    profLbl->setStyleSheet("font-weight: bold;");
    cfgLayout->addWidget(profLbl, 0, 0);
    auto *profRow = new QHBoxLayout;
    m_profileCombo = new QComboBox;
    m_profileCombo->addItem("(none)");
    m_profileCombo->setToolTip("Select a build profile from ul.toml");
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BuildTab::onProfileSelected);
    profRow->addWidget(m_profileCombo, 1);
    auto *saveProfBtn = new QPushButton("Save Profile");
    saveProfBtn->setToolTip("Save current settings as a named profile");
    connect(saveProfBtn, &QPushButton::clicked, this, &BuildTab::saveProfile);
    profRow->addWidget(saveProfBtn);
    auto *delProfBtn = new QPushButton("Delete");
    delProfBtn->setToolTip("Delete the selected profile");
    connect(delProfBtn, &QPushButton::clicked, this, &BuildTab::deleteProfile);
    profRow->addWidget(delProfBtn);
    cfgLayout->addLayout(profRow, 0, 1);

    // Row 1: Source File
    auto *srcLbl = new QLabel("Source File:");
    srcLbl->setStyleSheet("font-weight: bold;");
    cfgLayout->addWidget(srcLbl, 1, 0);

    auto *srcRow = new QHBoxLayout;
    m_srcEdit = new QLineEdit;
    srcRow->addWidget(m_srcEdit);
    auto *browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &BuildTab::browseSource);
    srcRow->addWidget(browseBtn);
    cfgLayout->addLayout(srcRow, 1, 1);

    // Row 2: Target
    auto *targetLbl = new QLabel("Target:");
    targetLbl->setStyleSheet("font-weight: bold;");
    cfgLayout->addWidget(targetLbl, 2, 0);

    auto *targetRow = new QHBoxLayout;
    m_targetGroup = new QButtonGroup(this);

    struct TargetOpt { QString val; QString label; };
    QVector<TargetOpt> targets = {
        {"c", "C"}, {"llvm", "LLVM IR"}, {"python", "Python"}, {"js", "JavaScript"}
    };

    for (const auto &t : targets) {
        auto *rb = new QRadioButton(t.label);
        rb->setChecked(t.val == "c");
        connect(rb, &QRadioButton::toggled, this,
                [this, val = t.val](bool checked) {
                    if (checked) m_targetValue = val;
                });
        m_targetGroup->addButton(rb);
        targetRow->addWidget(rb);
    }
    targetRow->addStretch();
    cfgLayout->addLayout(targetRow, 2, 1);

    // Row 3: Output
    auto *outLbl = new QLabel("Output:");
    outLbl->setStyleSheet("font-weight: bold;");
    cfgLayout->addWidget(outLbl, 3, 0);
    m_outEdit = new QLineEdit("(auto)");
    m_outEdit->setPlaceholderText("(auto) or specify output path");
    cfgLayout->addWidget(m_outEdit, 3, 1);

    layout->addWidget(cfgGroup);

    // ── Build / Run button row ──────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    m_buildBtn = new QPushButton("Build");
    m_buildBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(m_buildBtn, &QPushButton::clicked, this, &BuildTab::build);
    btnRow->addWidget(m_buildBtn);

    m_runBtn = new QPushButton("Run");
    m_runBtn->setToolTip("Run the compiled program (python Main.py run <src>)");
    connect(m_runBtn, &QPushButton::clicked, this, &BuildTab::runProgram);
    btnRow->addWidget(m_runBtn);

    m_statusLbl = new QLabel;
    btnRow->addWidget(m_statusLbl);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    // ── Log panel ───────────────────────────────────────────────────────
    auto *logGroup  = new QGroupBox("Compiler Output");
    auto *logLayout = new QVBoxLayout(logGroup);

    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setFont(QFont("Consolas", 10));
    m_log->setMinimumHeight(200);
    logLayout->addWidget(m_log);

    // Log action buttons
    auto *logBtnRow = new QHBoxLayout;
    logBtnRow->addStretch();

    m_copyBtn = new QPushButton("Copy Log");
    m_copyBtn->setToolTip("Copy full log to clipboard");
    connect(m_copyBtn, &QPushButton::clicked, this, &BuildTab::copyLog);
    logBtnRow->addWidget(m_copyBtn);

    m_clearBtn = new QPushButton("Clear");
    m_clearBtn->setToolTip("Clear log output");
    connect(m_clearBtn, &QPushButton::clicked, this, &BuildTab::clearLog);
    logBtnRow->addWidget(m_clearBtn);

    logLayout->addLayout(logBtnRow);

    layout->addWidget(logGroup, 1);
}

void BuildTab::setActionButtonsEnabled(bool enabled)
{
    m_buildBtn->setEnabled(enabled);
    m_runBtn->setEnabled(enabled);
}

void BuildTab::onThemeChanged(ThemeMode /*mode*/, const ThemeColors &colors)
{
    m_colors = colors;
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    m_buildBtn->setStyleSheet(navyBtnStyle(m_colors));
}

void BuildTab::browseSource()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Select .ul file", QString(), "UniLogic (*.ul);;All Files (*)");
    if (!path.isEmpty())
        m_srcEdit->setText(QDir::toNativeSeparators(path));
}

void BuildTab::onLogLine(const QString &text, const QString &tag)
{
    QString color;
    QString displayText = text;

    if (tag == "error")        color = m_colors.red;
    else if (tag == "warning") color = m_colors.orange;
    else if (tag == "success") color = m_colors.green;
    else if (tag == "info")    color = m_colors.blue;
    else if (tag == "cmd")     color = m_colors.textDim;
    else                       color = m_colors.text;

    // Enhanced error recognition for specific patterns
    if (text.contains("killswitch failed")) {
        color = m_colors.red;
        displayText = QStringLiteral("[ASSERTION FAILED] %1").arg(text);
    } else if (text.contains("catch") && text.contains("error")) {
        color = m_colors.orange;
    } else if (text.contains("goto") && text.contains("not found")) {
        color = m_colors.red;
        displayText = QStringLiteral("[PORTAL NOT FOUND] %1").arg(text);
    }

    m_log->append(QStringLiteral(
        "<span style=\"color:%1; font-family:Consolas;\">%2</span>")
        .arg(color, displayText.toHtmlEscaped()));

    auto *sb = m_log->verticalScrollBar();
    sb->setValue(sb->maximum());

    // Forward to output panel
    if (m_isBuilding)
        emit buildLogLine(text, tag);
    else
        emit runLogLine(text, tag);

    // Parse build errors: filename.ul:LINE:COL — message
    if (m_isBuilding && (tag == "error" || tag == "warning"))
        parseBuildErrors(text);
}

void BuildTab::parseBuildErrors(const QString &text)
{
    static QRegularExpression rx(R"((\S+\.ul):(\d+):(\d+)\s*[-—:]\s*(.*))");
    auto match = rx.match(text);
    if (match.hasMatch()) {
        QString file = match.captured(1);
        int line = match.captured(2).toInt();
        int col  = match.captured(3).toInt();
        QString msg = match.captured(4);
        QString sev = text.toLower().contains("warning") ? "warning" : "error";
        emit buildProblem(file, line, col, sev, msg);
    }
}

void BuildTab::copyLog()
{
    QString plain = m_log->toPlainText();
    if (!plain.isEmpty())
        QApplication::clipboard()->setText(plain);
}

void BuildTab::clearLog()
{
    m_log->clear();
}

void BuildTab::build()
{
    // ── Read compiler directory from settings ───────────────────────────
    QString compilerDir = SettingsDialog::loadCompilerDir();
    if (compilerDir.isEmpty()) {
        QMessageBox::warning(this, "No Compiler Path",
            "No compiler path is configured.\n\n"
            "Open Settings (gear icon in the header) and set the path to "
            "the XPile directory containing Main.py.");
        return;
    }

    QString mainPy = QDir(compilerDir).absoluteFilePath("Main.py");
    if (!QFileInfo::exists(mainPy)) {
        QMessageBox::warning(this, "Main.py Not Found",
            QStringLiteral("Could not find Main.py in:\n%1\n\n"
                "Open Settings and correct the compiler path.")
                .arg(QDir::toNativeSeparators(compilerDir)));
        return;
    }

    // ── Validate source file ────────────────────────────────────────────
    QString src = m_srcEdit->text().trimmed();
    if (src.isEmpty() || !QFileInfo::exists(src)) {
        QMessageBox::warning(this, "No Source", "Select a valid .ul source file.");
        return;
    }

    // ── Kick off build ──────────────────────────────────────────────────
    m_log->clear();
    m_isBuilding = true;
    emit buildStarted();
    setActionButtonsEnabled(false);
    m_statusLbl->setText("Building...");
    m_statusLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.orange));

    QString out = m_outEdit->text().trimmed();

    m_worker = new BuildWorker(src, m_targetValue, out, compilerDir, this);

    onLogLine(QStringLiteral("> %1").arg(m_worker->fullCommand()), "cmd");
    onLogLine(QString(), "");

    connect(m_worker, &BuildWorker::logLine,       this, &BuildTab::onLogLine);
    connect(m_worker, &BuildWorker::finishedBuild,  this, &BuildTab::onBuildDone);
    connect(m_worker, &BuildWorker::finished,       m_worker, &QObject::deleteLater);
    m_worker->start();
}

void BuildTab::onBuildDone(int returnCode)
{
    m_isBuilding = false;
    setActionButtonsEnabled(true);
    if (returnCode == 0) {
        m_statusLbl->setText("Build succeeded");
        m_statusLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.green));
    } else {
        m_statusLbl->setText(QStringLiteral("Build failed (exit %1)").arg(returnCode));
        m_statusLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.red));
    }
}

void BuildTab::runProgram()
{
    QString compilerDir = SettingsDialog::loadCompilerDir();
    if (compilerDir.isEmpty()) {
        QMessageBox::warning(this, "No Compiler Path",
            "Set the compiler path in Settings first.");
        return;
    }

    QString src = m_srcEdit->text().trimmed();
    if (src.isEmpty() || !QFileInfo::exists(src)) {
        QMessageBox::warning(this, "No Source", "Select a valid .ul source file.");
        return;
    }

    m_log->clear();
    m_isBuilding = false;
    emit runStarted();
    setActionButtonsEnabled(false);
    m_statusLbl->setText("Running...");
    m_statusLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.blue));

    m_runWorker = new RunWorker(src, compilerDir, this);
    onLogLine(QStringLiteral("> %1").arg(m_runWorker->fullCommand()), "cmd");
    onLogLine(QString(), "");

    connect(m_runWorker, &RunWorker::logLine,      this, &BuildTab::onLogLine);
    connect(m_runWorker, &RunWorker::finishedRun,   this, &BuildTab::onRunDone);
    connect(m_runWorker, &QThread::finished,        m_runWorker, &QObject::deleteLater);
    m_runWorker->start();
}

void BuildTab::onRunDone(int returnCode)
{
    setActionButtonsEnabled(true);
    if (returnCode == 0) {
        m_statusLbl->setText("Run completed");
        m_statusLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.green));
    } else {
        m_statusLbl->setText(QStringLiteral("Run failed (exit %1)").arg(returnCode));
        m_statusLbl->setStyleSheet(
            QStringLiteral("color: %1;").arg(m_colors.red));
    }
}

// ── Build profiles ──────────────────────────────────────────────────────

void BuildTab::setProjectDir(const QString &dir)
{
    m_projectDir = dir;
    loadProfiles();
}

void BuildTab::loadProfiles()
{
    m_profiles.clear();
    m_profileCombo->clear();
    m_profileCombo->addItem("(none)");

    if (m_projectDir.isEmpty()) return;
    QString tomlPath = QDir(m_projectDir).filePath("ul.toml");
    QFile file(tomlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    // Simple TOML parser for [profiles.name] sections
    Profile current;
    bool inProfile = false;
    static QRegularExpression sectionRx(R"(\[profiles\.(\w+)\])");
    static QRegularExpression kvRx(R"xx((\w+)\s*=\s*"?([^"\s]+)"?)xx");

    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine()).trimmed();
        auto secMatch = sectionRx.match(line);
        if (secMatch.hasMatch()) {
            if (inProfile) m_profiles.append(current);
            current = Profile{secMatch.captured(1), "c", "gc", "checked", "(auto)", 0};
            inProfile = true;
            continue;
        }
        if (line.startsWith('[')) { if (inProfile) m_profiles.append(current); inProfile = false; continue; }
        if (!inProfile) continue;
        auto kv = kvRx.match(line);
        if (!kv.hasMatch()) continue;
        QString key = kv.captured(1), val = kv.captured(2);
        if (key == "target")     current.target = val;
        else if (key == "dr_memory") current.drMemory = val;
        else if (key == "dr_safety") current.drSafety = val;
        else if (key == "norm_level") current.normLevel = val.toInt();
        else if (key == "output") current.output = val;
    }
    if (inProfile) m_profiles.append(current);
    file.close();

    for (const auto &p : m_profiles)
        m_profileCombo->addItem(p.name);
}

void BuildTab::writeProfiles()
{
    if (m_projectDir.isEmpty()) return;
    QString tomlPath = QDir(m_projectDir).filePath("ul.toml");

    // Read existing non-profile content
    QString existing;
    QFile readFile(tomlPath);
    if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString all = QTextStream(&readFile).readAll();
        readFile.close();
        // Remove all [profiles.*] sections
        static QRegularExpression profBlockRx(R"(\[profiles\.\w+\][^\[]*)", QRegularExpression::DotMatchesEverythingOption);
        existing = all;
        existing.replace(profBlockRx, "");
        existing = existing.trimmed();
    }

    QFile file(tomlPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    if (!existing.isEmpty()) out << existing << "\n\n";
    for (const auto &p : m_profiles) {
        out << "[profiles." << p.name << "]\n";
        out << "target = \"" << p.target << "\"\n";
        out << "dr_memory = \"" << p.drMemory << "\"\n";
        out << "dr_safety = \"" << p.drSafety << "\"\n";
        out << "norm_level = " << p.normLevel << "\n";
        if (p.output != "(auto)") out << "output = \"" << p.output << "\"\n";
        out << "\n";
    }
    file.close();
}

void BuildTab::onProfileSelected(int index)
{
    if (index <= 0 || index > m_profiles.size()) return;
    const Profile &p = m_profiles[index - 1];
    // Set target radio
    for (auto *btn : m_targetGroup->buttons()) {
        auto *rb = qobject_cast<QRadioButton*>(btn);
        if (rb && rb->text().toLower().startsWith(p.target.left(1)))
            rb->setChecked(true);
    }
    if (p.output != "(auto)") m_outEdit->setText(p.output);
}

void BuildTab::saveProfile()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Save Profile", "Profile name:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    name = name.simplified().replace(' ', '_');

    Profile p{name, m_targetValue, "gc", "checked", m_outEdit->text().trimmed(), 0};
    // Replace if exists
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].name == name) { m_profiles[i] = p; writeProfiles(); loadProfiles(); return; }
    }
    m_profiles.append(p);
    writeProfiles();
    loadProfiles();
    m_profileCombo->setCurrentText(name);
}

void BuildTab::deleteProfile()
{
    int idx = m_profileCombo->currentIndex();
    if (idx <= 0 || idx > m_profiles.size()) return;
    m_profiles.removeAt(idx - 1);
    writeProfiles();
    loadProfiles();
}
