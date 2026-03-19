// drconfiguratortab.cpp -- "DR Config" tab implementation

#include "drconfiguratortab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>

struct DRSettingDef {
    QString key;
    QString label;
    QStringList options;
    QString shortDesc;
};

static const QVector<DRSettingDef> DR_SETTINGS = {
    {"memory",      "Memory Model",  {"manual", "gc", "refcount", "arena"},
     "Allocation strategy: manual, gc, refcount, arena"},
    {"safety",      "Safety Mode",   {"checked", "unchecked"},
     "checked: runtime assertions; unchecked: max perf"},
    {"types",       "Type System",   {"strict", "dynamic"},
     "strict: explicit types; dynamic: inference"},
    {"int_width",   "Integer Width", {"32", "64", "platform"},
     "Default bit-width for bare 'int' declarations"},
    {"concurrency", "Concurrency",   {"threaded", "parallel", "async", "cooperative"},
     "threaded, parallel, async, or cooperative"},
    {"precision",   "Precision",     {"fp32", "fp16", "bf16", "int8", "int4"},
     "Floating-point precision for AI/compute workloads"},
    {"compute",     "Compute",       {"cpu", "cuda", "rocm", "metal", "tensor"},
     "Hardware target for compute-intensive code"},
    {"vectorize",   "Vectorize",     {"auto", "avx2", "avx512", "neon", "sve"},
     "SIMD instruction set for vectorized operations"},
    {"sparsity",    "Sparsity",      {"dense", "sparse", "auto"},
     "Dense or sparse computation strategy"},
};

static const QStringList DR_SETTING_NAMES = {
    "memory", "safety", "types", "int_width", "concurrency",
    "precision", "compute", "vectorize", "sparsity"
};

static QMap<QString, QString> parseDrBlock(const QString &text)
{
    QMap<QString, QString> found;
    static QRegularExpression rx(R"((?://\s*)?@dr\s+(\w+)\s*=\s*(\w+))");
    auto it = rx.globalMatch(text);
    while (it.hasNext()) { auto m = it.next(); found[m.captured(1)] = m.captured(2); }
    return found;
}

// ── Conflict matrix ─────────────────────────────────────────────────────

struct ConflictRule {
    QString key1, val1, key2, val2;
    QString severity;   // "error" or "warning"
    QString message;
};

static const QVector<ConflictRule> CONFLICT_RULES = {
    {"memory", "gc",      "concurrency", "threaded", "warning",
     "GC stop-the-world pauses block all threads"},
    {"memory", "gc",      "concurrency", "parallel", "warning",
     "GC pauses stall parallel workers"},
    {"memory", "manual",  "safety", "unchecked", "warning",
     "Manual memory + unchecked = no safety net for use-after-free"},
    {"memory", "arena",   "concurrency", "async", "error",
     "Arena regions can't span async boundaries safely"},
    {"safety", "unchecked", "types", "dynamic", "error",
     "Unchecked + dynamic typing removes all runtime safety"},
    {"memory", "refcount", "concurrency", "parallel", "warning",
     "Refcount contention on shared objects in parallel code"},
    // AI/Performance conflicts
    {"compute", "cuda",    "memory", "gc", "warning",
     "GC pauses will interrupt GPU computation — use arena or manual"},
    {"compute", "rocm",    "memory", "gc", "warning",
     "GC pauses will interrupt GPU computation — use arena or manual"},
    {"compute", "tensor",  "memory", "gc", "warning",
     "GC pauses will interrupt tensor accelerator — use arena or manual"},
    {"precision", "int4",  "safety", "checked", "warning",
     "int4 precision with checked safety adds overflow checking overhead"},
    {"precision", "int8",  "safety", "checked", "warning",
     "int8 precision with checked safety adds overflow checking overhead"},
    {"compute", "cuda",    "concurrency", "cooperative", "error",
     "CUDA requires threaded or parallel concurrency, not cooperative"},
    {"sparsity", "sparse", "vectorize", "avx512", "warning",
     "Sparse mode reduces effectiveness of wide SIMD vectorization"},
};

// ── Constructor ─────────────────────────────────────────────────────────

DRConfiguratorTab::DRConfiguratorTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    // Header
    m_titleLbl = new QLabel("DR Configurator");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    layout->addWidget(m_titleLbl);

    m_subtitleLbl = new QLabel(
        "Configure Dynamic Runtime settings and write the DR header block into a .ul file.");
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
    layout->addWidget(m_subtitleLbl);

    // File selector
    auto *fileRow = new QHBoxLayout;
    auto *fileLbl = new QLabel("Target File:");
    fileLbl->setStyleSheet("font-weight: bold;");
    fileRow->addWidget(fileLbl);
    m_fileEdit = new QLineEdit;
    fileRow->addWidget(m_fileEdit, 1);
    auto *browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &DRConfiguratorTab::browseFile);
    fileRow->addWidget(browseBtn);
    auto *loadBtn = new QPushButton("Load");
    connect(loadBtn, &QPushButton::clicked, this, &DRConfiguratorTab::loadFile);
    fileRow->addWidget(loadBtn);
    layout->addLayout(fileRow);

    // Preset buttons
    auto *presetRow = new QHBoxLayout;
    auto *presetLbl = new QLabel("Presets:");
    presetLbl->setStyleSheet("font-weight: bold;");
    presetRow->addWidget(presetLbl);
    for (const QString &name : {"Safe Default", "High Performance",
                                 "Embedded/Realtime", "AI Inference"}) {
        auto *btn = new QPushButton(name);
        btn->setStyleSheet(navyBtnStyle(m_colors));
        connect(btn, &QPushButton::clicked, this,
                [this, name]() { applyPreset(name); });
        presetRow->addWidget(btn);
    }
    auto *resetBtn = new QPushButton("Reset");
    connect(resetBtn, &QPushButton::clicked, this, &DRConfiguratorTab::resetDefaults);
    presetRow->addWidget(resetBtn);
    presetRow->addStretch();
    layout->addLayout(presetRow);

    // Settings grid
    auto *settingsGroup  = new QGroupBox("DR Settings");
    auto *settingsLayout = new QGridLayout(settingsGroup);
    settingsLayout->setColumnStretch(3, 1);
    settingsLayout->setVerticalSpacing(10);
    settingsLayout->setHorizontalSpacing(10);

    for (int i = 0; i < DR_SETTINGS.size(); ++i) {
        const auto &s = DR_SETTINGS[i];

        auto *cb = new QCheckBox;
        cb->setChecked(false);
        m_enableChecks[s.key] = cb;
        settingsLayout->addWidget(cb, i, 0);

        auto *lbl = new QLabel(s.label);
        lbl->setStyleSheet("font-weight: bold;");
        lbl->setFixedWidth(130);
        settingsLayout->addWidget(lbl, i, 1);

        auto *combo = new QComboBox;
        combo->addItems(s.options);
        combo->setFixedWidth(150);
        m_settingCombos[s.key] = combo;
        settingsLayout->addWidget(combo, i, 2);

        auto *descLbl = new QLabel(s.shortDesc);
        descLbl->setStyleSheet(
            QStringLiteral("color: %1; font-size: 9pt;").arg(m_colors.textDim));
        settingsLayout->addWidget(descLbl, i, 3);

        connect(cb, &QCheckBox::checkStateChanged, this, &DRConfiguratorTab::updatePreview);
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &DRConfiguratorTab::updatePreview);
    }
    layout->addWidget(settingsGroup);

    // Conflict label
    m_conflictLbl = new QLabel;
    m_conflictLbl->setWordWrap(true);
    m_conflictLbl->setStyleSheet(
        QStringLiteral("font-size: 10pt; padding: 4px; background: transparent;"));
    layout->addWidget(m_conflictLbl);

    // Preview
    auto *prevLbl = new QLabel("Preview:");
    prevLbl->setStyleSheet("font-weight: bold;");
    layout->addWidget(prevLbl);

    m_preview = new QPlainTextEdit;
    m_preview->setReadOnly(true);
    m_preview->setFont(QFont("Consolas", 10));
    m_preview->setMaximumHeight(120);
    layout->addWidget(m_preview);
    updatePreview();

    // Write button
    auto *btnRow = new QHBoxLayout;
    auto *writeBtn = new QPushButton("Write to File");
    writeBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(writeBtn, &QPushButton::clicked, this, &DRConfiguratorTab::writeToFile);
    btnRow->addWidget(writeBtn);
    m_writeStatus = new QLabel;
    btnRow->addWidget(m_writeStatus);
    btnRow->addStretch();
    layout->addLayout(btnRow);
    layout->addStretch();
}

void DRConfiguratorTab::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 14pt; font-weight: bold; color: %1;")
            .arg(m_colors.navyLight));
    m_subtitleLbl->setStyleSheet(
        QStringLiteral("color: %1;").arg(m_colors.textDim));
}

void DRConfiguratorTab::applyPreset(const QString &presetName)
{
    auto setVal = [&](const QString &key, const QString &val) {
        if (!m_enableChecks.contains(key)) return;
        m_enableChecks[key]->setChecked(true);
        int idx = m_settingCombos[key]->findText(val);
        if (idx >= 0) m_settingCombos[key]->setCurrentIndex(idx);
    };

    if (presetName == "Safe Default") {
        setVal("memory", "gc"); setVal("safety", "checked");
        setVal("types", "strict"); setVal("int_width", "64"); setVal("concurrency", "threaded");
    } else if (presetName == "High Performance") {
        setVal("memory", "manual"); setVal("safety", "unchecked");
        setVal("types", "strict"); setVal("int_width", "64"); setVal("concurrency", "parallel");
    } else if (presetName == "Embedded/Realtime") {
        setVal("memory", "arena"); setVal("safety", "checked");
        setVal("types", "strict"); setVal("int_width", "32"); setVal("concurrency", "cooperative");
    } else if (presetName == "AI Inference") {
        setVal("memory", "arena"); setVal("safety", "unchecked");
        setVal("types", "strict"); setVal("int_width", "64"); setVal("concurrency", "parallel");
        setVal("precision", "fp16"); setVal("compute", "cpu");
        setVal("vectorize", "auto"); setVal("sparsity", "dense");
    }
}

void DRConfiguratorTab::resetDefaults()
{
    for (auto it = m_enableChecks.begin(); it != m_enableChecks.end(); ++it)
        it.value()->setChecked(false);
    for (auto it = m_settingCombos.begin(); it != m_settingCombos.end(); ++it)
        it.value()->setCurrentIndex(0);
}

void DRConfiguratorTab::checkConflicts()
{
    QStringList warnings, errors;

    for (const auto &rule : CONFLICT_RULES) {
        if (!m_enableChecks[rule.key1]->isChecked()) continue;
        if (!m_enableChecks[rule.key2]->isChecked()) continue;
        if (m_settingCombos[rule.key1]->currentText() != rule.val1) continue;
        if (m_settingCombos[rule.key2]->currentText() != rule.val2) continue;

        QString msg = QStringLiteral("%1=%2 + %3=%4: %5")
            .arg(rule.key1, rule.val1, rule.key2, rule.val2, rule.message);
        if (rule.severity == "error")
            errors << msg;
        else
            warnings << msg;
    }

    if (errors.isEmpty() && warnings.isEmpty()) {
        m_conflictLbl->setText("");
        return;
    }

    QStringList parts;
    for (const QString &e : errors)
        parts << QStringLiteral("<span style='color:%1'>Error: %2</span>")
                     .arg(m_colors.red, e.toHtmlEscaped());
    for (const QString &w : warnings)
        parts << QStringLiteral("<span style='color:%1'>Warning: %2</span>")
                     .arg(m_colors.orange, w.toHtmlEscaped());

    m_conflictLbl->setText(parts.join("<br>"));
}

void DRConfiguratorTab::browseFile()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Select .ul file", QString(), "UniLogic (*.ul);;All Files (*)");
    if (!path.isEmpty())
        m_fileEdit->setText(QDir::toNativeSeparators(path));
}

void DRConfiguratorTab::loadFile()
{
    QString path = m_fileEdit->text().trimmed();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        QMessageBox::warning(this, "File Error", "Select a valid .ul file.");
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QString text = QTextStream(&file).readAll();
    file.close();

    auto found = parseDrBlock(text);
    for (auto it = m_enableChecks.begin(); it != m_enableChecks.end(); ++it)
        it.value()->setChecked(false);
    for (auto it = found.cbegin(); it != found.cend(); ++it) {
        if (m_enableChecks.contains(it.key())) {
            m_enableChecks[it.key()]->setChecked(true);
            int idx = m_settingCombos[it.key()]->findText(it.value());
            if (idx >= 0) m_settingCombos[it.key()]->setCurrentIndex(idx);
        }
    }
    int count = found.size();
    m_writeStatus->setText(QStringLiteral("Loaded %1 DR setting%2 from file.")
        .arg(count).arg(count != 1 ? "s" : ""));
    m_writeStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.navyLight));
}

QStringList DRConfiguratorTab::getDrLines() const
{
    QStringList lines;
    for (const auto &s : DR_SETTINGS) {
        if (m_enableChecks[s.key]->isChecked()) {
            QString val = m_settingCombos[s.key]->currentText();
            lines << QStringLiteral("@dr %1 = %2").arg(s.key, val);
        }
    }
    return lines;
}

void DRConfiguratorTab::updatePreview()
{
    QStringList lines = getDrLines();
    if (!lines.isEmpty())
        m_preview->setPlainText("// DR Configuration\n" + lines.join("\n"));
    else
        m_preview->setPlainText("(no DR settings enabled)");
    checkConflicts();
}

void DRConfiguratorTab::writeToFile()
{
    QString path = m_fileEdit->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "No File", "Select a target .ul file first.");
        return;
    }
    QStringList drLines = getDrLines();
    if (drLines.isEmpty()) {
        QMessageBox::information(this, "Nothing to Write",
                                 "Enable at least one DR setting.");
        return;
    }
    QString existing;
    QFile file(path);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            existing = QTextStream(&file).readAll();
            file.close();
        }
    }
    static QRegularExpression drBlockRx(
        R"((//\s*DR Configuration\n)?((?://\s*)?@dr\s+\w+\s*=\s*\w+\n)*)");
    QString cleaned = existing;
    cleaned.replace(drBlockRx, QString());
    while (cleaned.startsWith('\n')) cleaned.remove(0, 1);

    QString header = "// DR Configuration\n" + drLines.join("\n") + "\n\n";
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << header << cleaned;
        file.close();
        QFileInfo fi(path);
        m_writeStatus->setText(QStringLiteral("Wrote %1 settings to %2")
            .arg(drLines.size()).arg(fi.fileName()));
        m_writeStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.green));
    } else {
        m_writeStatus->setText("Error: could not write file");
        m_writeStatus->setStyleSheet(QStringLiteral("color: %1;").arg(m_colors.red));
    }
}
