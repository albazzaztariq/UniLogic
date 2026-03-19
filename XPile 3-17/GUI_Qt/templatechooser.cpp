#include "templatechooser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QLabel>

TemplateChooser::TemplateChooser(const ThemeColors &colors, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("New File from Template");
    resize(600, 400);

    m_templates = {
        {"Empty UL file", ""},
        {"Function file",
         "// Function module\n\nfunction myFunction(x int) returns int\n  return x\nend function\n"},
        {"Type definition",
         "// Type definition\n\ntype Point\n  x float\n  y float\nend type\n"},
        {"Object definition",
         "// Object definition\n\nobject Counter\n  count int\n\n  function increment() returns int\n    count = count + 1\n    return count\n  end function\nend object\n"},
        {"Test file",
         "// Test file\n\nfunction main() returns int\n  // Test setup\n  print \"Running tests...\"\n\n  // Test 1\n  print \"Test 1: PASS\"\n\n  print \"All tests passed\"\n  return 0\nend function\n"},
        {"DR configured file",
         "// DR Configuration\n@dr memory = gc\n@dr safety = checked\n@dr types = strict\n@dr int_width = 64\n@dr concurrency = threaded\n\nfunction main() returns int\n  return 0\nend function\n"},
        {"AI/ML file",
         "// AI/ML workload configuration\n"
         "// precision: fp16 gives 2x throughput on most GPUs vs fp32\n"
         "// compute: cpu for portability, cuda/rocm/metal for GPU acceleration\n"
         "// memory: arena avoids GC pauses during batch inference\n"
         "// vectorize: auto lets the compiler pick the best SIMD instructions\n"
         "@dr precision = fp16\n"
         "@dr compute = cpu\n"
         "@dr memory = arena\n"
         "@dr safety = unchecked\n"
         "@dr vectorize = auto\n"
         "@dr sparsity = dense\n"
         "@dr int_width = 64\n"
         "@dr concurrency = parallel\n\n"
         "function main() returns int\n"
         "  print \"AI/ML workload ready\"\n"
         "  return 0\n"
         "end function\n"},
        {"Server file",
         "// Server application configuration\n"
         "// memory: manual for predictable latency (no GC pauses)\n"
         "// concurrency: threaded for handling concurrent connections\n"
         "// safety: checked for production reliability\n"
         "@dr memory = manual\n"
         "@dr concurrency = threaded\n"
         "@dr safety = checked\n"
         "@dr types = strict\n"
         "@dr int_width = 64\n\n"
         "function main() returns int\n"
         "  print \"Server starting...\"\n"
         "  return 0\n"
         "end function\n"},
    };

    auto *layout = new QVBoxLayout(this);
    auto *titleLbl = new QLabel("Choose a template:");
    titleLbl->setStyleSheet(QStringLiteral("font-weight: bold; color: %1;").arg(colors.navyLight));
    layout->addWidget(titleLbl);

    auto *splitter = new QSplitter(Qt::Horizontal);

    m_list = new QListWidget;
    for (const auto &t : m_templates)
        m_list->addItem(t.name);
    connect(m_list, &QListWidget::currentRowChanged, this, [this](int) { onSelectionChanged(); });
    splitter->addWidget(m_list);

    m_preview = new QPlainTextEdit;
    m_preview->setReadOnly(true);
    m_preview->setFont(QFont("Consolas", 10));
    splitter->addWidget(m_preview);
    splitter->setSizes({200, 400});

    layout->addWidget(splitter, 1);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton("Cancel");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancelBtn);
    auto *createBtn = new QPushButton("Create");
    createBtn->setStyleSheet(navyBtnStyle(colors));
    connect(createBtn, &QPushButton::clicked, this, [this]() {
        int row = m_list->currentRow();
        if (row >= 0 && row < m_templates.size()) {
            m_content = m_templates[row].content;
            m_name = m_templates[row].name;
        }
        accept();
    });
    btnRow->addWidget(createBtn);
    layout->addLayout(btnRow);

    if (m_list->count() > 0) m_list->setCurrentRow(0);
}

void TemplateChooser::onSelectionChanged()
{
    int row = m_list->currentRow();
    if (row >= 0 && row < m_templates.size())
        m_preview->setPlainText(m_templates[row].content.isEmpty() ? "(empty file)" : m_templates[row].content);
}
