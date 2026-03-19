#include "normdiffviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>

NormDiffViewer::NormDiffViewer(const QString &filePath, const QString &compilerDir,
                               const ThemeColors &colors, QWidget *parent)
    : QDialog(parent), m_colors(colors), m_filePath(filePath), m_compilerDir(compilerDir)
{
    setWindowTitle("Normalization Diff");
    resize(900, 600);

    auto *layout = new QVBoxLayout(this);

    auto *sliderRow = new QHBoxLayout;
    sliderRow->addWidget(new QLabel("Normalization Level:"));
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 3);
    m_slider->setValue(0);
    sliderRow->addWidget(m_slider);
    m_levelLbl = new QLabel("0");
    m_levelLbl->setFixedWidth(20);
    sliderRow->addWidget(m_levelLbl);
    layout->addLayout(sliderRow);

    auto *splitter = new QSplitter(Qt::Horizontal);
    m_leftPanel = new QPlainTextEdit;
    m_leftPanel->setReadOnly(true);
    m_leftPanel->setFont(QFont("Consolas", 10));
    m_rightPanel = new QPlainTextEdit;
    m_rightPanel->setReadOnly(true);
    m_rightPanel->setFont(QFont("Consolas", 10));
    splitter->addWidget(m_leftPanel);
    splitter->addWidget(m_rightPanel);
    layout->addWidget(splitter, 1);

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_originalText = QTextStream(&file).readAll();
        file.close();
    }
    m_leftPanel->setPlainText(m_originalText);

    m_debounce.setSingleShot(true);
    m_debounce.setInterval(300);
    connect(&m_debounce, &QTimer::timeout, this, &NormDiffViewer::runNormalization);
    connect(m_slider, &QSlider::valueChanged, this, &NormDiffViewer::onSliderChanged);

    runNormalization();
}

void NormDiffViewer::onSliderChanged(int value)
{
    m_levelLbl->setText(QString::number(value));
    m_debounce.start();
}

void NormDiffViewer::runNormalization()
{
    int level = m_slider->value();
    QString mainPy = QDir(m_compilerDir).absoluteFilePath("Main.py");
    QProcess proc;
    proc.start("python", {mainPy, m_filePath, "--norm-level", QString::number(level), "--dry-run"});
    if (!proc.waitForFinished(10000)) { m_rightPanel->setPlainText("[timeout]"); return; }

    QString normalized = QString::fromUtf8(proc.readAllStandardOutput());
    if (normalized.isEmpty()) normalized = QString::fromUtf8(proc.readAllStandardError());
    m_rightPanel->setPlainText(normalized);

    // Highlight differences
    QStringList origLines = m_originalText.split('\n');
    QStringList normLines = normalized.split('\n');
    QString html;
    int maxLines = qMax(origLines.size(), normLines.size());
    for (int i = 0; i < maxLines; ++i) {
        QString orig = i < origLines.size() ? origLines[i] : "";
        QString norm = i < normLines.size() ? normLines[i] : "";
        if (orig != norm)
            html += QStringLiteral("<span style=\"background:%1\">%2</span><br>")
                .arg(m_colors.orange, norm.toHtmlEscaped());
        else
            html += norm.toHtmlEscaped() + "<br>";
    }
    m_rightPanel->clear();
    m_rightPanel->appendHtml(html);
}
