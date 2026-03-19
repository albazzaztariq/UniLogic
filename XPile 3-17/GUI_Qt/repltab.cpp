#include "repltab.h"
#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDir>
#include <QSettings>
#include <QKeyEvent>
#include <QScrollBar>
#include <QLabel>

static const char *KEY_REPL_HIST = "repl/history";

// Custom line edit to capture Up/Down arrows
class ReplLineEdit : public QLineEdit {
public:
    std::function<void()> onUp, onDown;
    using QLineEdit::QLineEdit;
protected:
    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Up && onUp) { onUp(); return; }
        if (e->key() == Qt::Key_Down && onDown) { onDown(); return; }
        QLineEdit::keyPressEvent(e);
    }
};

ReplTab::ReplTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_output = new QPlainTextEdit;
    m_output->setReadOnly(true);
    m_output->setFont(QFont("Consolas", 11));
    m_output->setStyleSheet(QStringLiteral(
        "QPlainTextEdit { background: #0a0a0e; color: %1; border: none; }").arg(m_colors.text));
    m_output->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    layout->addWidget(m_output, 1);

    // Banner
    appendOutput(QStringLiteral("\u221e UniLogic REPL"), m_colors.navyLight);
    appendOutput("One Language for Everything", m_colors.textDim);
    appendOutput("Type help for commands\n", m_colors.textDim);

    // Input row
    auto *inputRow = new QHBoxLayout;
    inputRow->setContentsMargins(4, 4, 4, 4);
    inputRow->setSpacing(4);

    auto *prompt = new QLabel("ul>");
    prompt->setStyleSheet(QStringLiteral(
        "color: %1; font-family: Consolas; font-size: 11pt; font-weight: bold; background: transparent; padding: 0 4px;")
        .arg(m_colors.navyLight));
    inputRow->addWidget(prompt);

    auto *lineEdit = new ReplLineEdit;
    lineEdit->setFont(QFont("Consolas", 11));
    lineEdit->setPlaceholderText("Enter UL expression...");
    lineEdit->onUp = [this]() { historyUp(); };
    lineEdit->onDown = [this]() { historyDown(); };
    m_input = lineEdit;
    connect(m_input, &QLineEdit::returnPressed, this, &ReplTab::executeInput);
    inputRow->addWidget(m_input, 1);

    m_runBtn = new QPushButton("Run");
    m_runBtn->setStyleSheet(navyBtnStyle(m_colors));
    connect(m_runBtn, &QPushButton::clicked, this, &ReplTab::executeInput);
    inputRow->addWidget(m_runBtn);

    m_clearBtn = new QPushButton("Clear");
    connect(m_clearBtn, &QPushButton::clicked, this, &ReplTab::clearOutput);
    inputRow->addWidget(m_clearBtn);

    layout->addLayout(inputRow);

    // Load history
    QSettings s(SettingsDialog::SETTINGS_ORG, SettingsDialog::SETTINGS_APP);
    m_history = s.value(KEY_REPL_HIST).toStringList();

    startProcess();
}

ReplTab::~ReplTab()
{
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void ReplTab::startProcess()
{
    QString compilerDir = SettingsDialog::loadCompilerDir();
    if (compilerDir.isEmpty()) return;

    QString mainPy = QDir(compilerDir).absoluteFilePath("Main.py");
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ReplTab::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,  this, &ReplTab::onProcessError);
    m_process->start("python", {mainPy, "repl"});
}

void ReplTab::executeInput()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;

    appendOutput(QStringLiteral("ul> %1").arg(text), m_colors.navyLight);

    // Handle local commands
    if (text == "help") {
        appendOutput("Commands: help, clear, exit", m_colors.textDim);
        appendOutput("", m_colors.textDim);
        appendOutput("Built-in functions:", m_colors.navyLight);
        appendOutput("  I/O:      print, userinput(), prompt(msg)", m_colors.text);
        appendOutput("  Math:     absval(x), random(), random_int(lo, hi), random_seed(n)", m_colors.text);
        appendOutput("  Time:     time(), clock(), sleep(ms)", m_colors.text);
        appendOutput("  Array:    size(arr), sort(arr), zip(a, b), enumerate(arr)", m_colors.text);
        appendOutput("  Higher:   map_fn(f, arr), filter_fn(f, arr)", m_colors.text);
        appendOutput("  Memory:   malloc(n), free(ptr), memmove, memcopy, memset", m_colors.text);
        appendOutput("  Control:  exit(code), abort(), vault(name)", m_colors.text);
        appendOutput("  Thread:   spawn(f), wait(), lock(m), unlock(m)", m_colors.text);
        m_input->clear();
        return;
    }
    if (text == "clear") { clearOutput(); m_input->clear(); return; }

    // Save to history
    m_history.removeAll(text);
    m_history.prepend(text);
    while (m_history.size() > 50) m_history.removeLast();
    m_historyIdx = -1;
    QSettings s(SettingsDialog::SETTINGS_ORG, SettingsDialog::SETTINGS_APP);
    s.setValue(KEY_REPL_HIST, m_history);

    // Send to process
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->write((text + "\n").toUtf8());
    } else {
        // Fallback: run as one-shot
        QString compilerDir = SettingsDialog::loadCompilerDir();
        if (compilerDir.isEmpty()) {
            appendOutput("[error] No compiler path set.", m_colors.red);
        } else {
            QString mainPy = QDir(compilerDir).absoluteFilePath("Main.py");
            QProcess oneshot;
            oneshot.start("python", {mainPy, "--run", text});
            oneshot.waitForFinished(10000);
            QString out = QString::fromUtf8(oneshot.readAllStandardOutput()).trimmed();
            QString err = QString::fromUtf8(oneshot.readAllStandardError()).trimmed();
            if (!out.isEmpty()) appendOutput(out, m_colors.text);
            if (!err.isEmpty()) appendOutput(err, m_colors.red);
        }
    }
    m_input->clear();
}

void ReplTab::onProcessOutput()
{
    QString text = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    if (!text.isEmpty()) appendOutput(text, m_colors.text);
}

void ReplTab::onProcessError()
{
    QString text = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    if (!text.isEmpty()) appendOutput(text, m_colors.red);
}

void ReplTab::clearOutput()
{
    m_output->clear();
    appendOutput(QStringLiteral("\u221e UniLogic REPL"), m_colors.navyLight);
    appendOutput("Type help for commands\n", m_colors.textDim);
}

void ReplTab::historyUp()
{
    if (m_history.isEmpty()) return;
    m_historyIdx = qMin(m_historyIdx + 1, m_history.size() - 1);
    m_input->setText(m_history[m_historyIdx]);
}

void ReplTab::historyDown()
{
    if (m_historyIdx <= 0) { m_historyIdx = -1; m_input->clear(); return; }
    m_historyIdx--;
    m_input->setText(m_history[m_historyIdx]);
}

void ReplTab::appendOutput(const QString &text, const QString &color)
{
    m_output->appendHtml(QStringLiteral(
        "<span style=\"color:%1; font-family:Consolas;\">%2</span>")
        .arg(color, text.toHtmlEscaped()));
    auto *sb = m_output->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void ReplTab::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    m_output->setStyleSheet(QStringLiteral(
        "QPlainTextEdit { background: #0a0a0e; color: %1; border: none; }").arg(m_colors.text));
    m_runBtn->setStyleSheet(navyBtnStyle(m_colors));
}
