// editortab.cpp -- Code editor tab implementation

#include "editortab.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QTextBlock>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcut>
#include <QScrollBar>
#include <QKeyEvent>

// ── ULHighlighter ───────────────────────────────────────────────────────

ULHighlighter::ULHighlighter(const ThemeColors &colors, QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    buildRules(colors);
}

void ULHighlighter::updateColors(const ThemeColors &colors)
{
    buildRules(colors);
    rehighlight();
}

void ULHighlighter::buildRules(const ThemeColors &c)
{
    m_rules.clear();

    // Keywords — navy blue bold
    QTextCharFormat kwFmt;
    kwFmt.setForeground(QColor(c.navyLight));
    kwFmt.setFontWeight(QFont::Bold);
    const QStringList keywords = {
        "function", "if", "while", "for", "end", "return", "print",
        "type", "object", "import", "fixed", "yields", "ok", "error",
        "cast", "exit", "else", "then", "do", "returns", "and", "or", "not"
    };
    for (const QString &kw : keywords)
        m_rules.append({QRegularExpression("\\b" + kw + "\\b"), kwFmt});

    // Types — purple
    QTextCharFormat typeFmt;
    typeFmt.setForeground(QColor("#c586c0"));
    const QStringList types = {"int", "float", "string", "bool", "none", "array"};
    for (const QString &t : types)
        m_rules.append({QRegularExpression("\\b" + t + "\\b"), typeFmt});

    // DR directives — teal
    QTextCharFormat drFmt;
    drFmt.setForeground(QColor(c.green));
    m_rules.append({QRegularExpression(R"(@(?:dr|norm|asm|deprecated|fuse|prefetch)\b)"), drFmt});

    // Numbers — green
    QTextCharFormat numFmt;
    numFmt.setForeground(QColor("#b5cea8"));
    m_rules.append({QRegularExpression(R"(\b\d+(?:\.\d+)?\b)"), numFmt});

    // Strings — amber/orange
    QTextCharFormat strFmt;
    strFmt.setForeground(QColor(c.orange));
    m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), strFmt});
    m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), strFmt});

    // Comments — grey italic
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor(c.textDim));
    commentFmt.setFontItalic(true);
    m_rules.append({QRegularExpression(R"(//.*)"), commentFmt});
}

void ULHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

// ── LineNumberArea ──────────────────────────────────────────────────────

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

// ── CodeEditor ──────────────────────────────────────────────────────────

CodeEditor::CodeEditor(const ThemeColors &colors, QWidget *parent)
    : QPlainTextEdit(parent), m_colors(colors)
{
    m_lineNumberArea = new LineNumberArea(this);

    setFont(QFont("Consolas", 12));
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_highlighter = new ULHighlighter(colors, document());

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);
    connect(document(), &QTextDocument::modificationChanged,
            this, &CodeEditor::modificationChanged);

    m_errorTimer.setSingleShot(true);
    connect(&m_errorTimer, &QTimer::timeout, this, &CodeEditor::clearErrorHighlight);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void CodeEditor::setColors(const ThemeColors &colors)
{
    m_colors = colors;
    m_highlighter->updateColors(colors);
    highlightCurrentLine();
    m_lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) { max /= 10; ++digits; }
    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                         lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(QColor(m_colors.bgRaised));
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.cursor = textCursor();
    sel.cursor.clearSelection();
    selections.append(sel);

    // Error line highlight (red tint)
    if (m_errorLine > 0) {
        QTextBlock block = document()->findBlockByLineNumber(m_errorLine - 1);
        if (block.isValid()) {
            QTextEdit::ExtraSelection errSel;
            errSel.format.setBackground(QColor(m_colors.red).lighter(180));
            errSel.format.setProperty(QTextFormat::FullWidthSelection, true);
            errSel.cursor = QTextCursor(block);
            errSel.cursor.clearSelection();
            selections.append(errSel);
        }
    }

    setExtraSelections(selections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(m_colors.bgSurface));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(m_colors.textDim));
            painter.drawText(0, top, m_lineNumberArea->width() - 4,
                             fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::goToLine(int line)
{
    QTextBlock block = document()->findBlockByLineNumber(line - 1);
    if (block.isValid()) {
        QTextCursor cursor(block);
        setTextCursor(cursor);
        centerCursor();
        setFocus();
    }
}

void CodeEditor::highlightErrorLine(int line)
{
    m_errorLine = line;
    goToLine(line);
    highlightCurrentLine();
    m_errorTimer.start(3000);
}

void CodeEditor::clearErrorHighlight()
{
    m_errorLine = -1;
    highlightCurrentLine();
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    // Ctrl+S — save handled by parent EditorTab shortcut
    QPlainTextEdit::keyPressEvent(event);
}

// ── EditorTab ───────────────────────────────────────────────────────────

EditorTab::EditorTab(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_subTabs = new QTabWidget;
    m_subTabs->setTabsClosable(true);
    m_subTabs->setMovable(true);
    m_subTabs->setDocumentMode(true);
    connect(m_subTabs, &QTabWidget::tabCloseRequested,
            this, &EditorTab::closeTab);

    layout->addWidget(m_subTabs);

    // Shortcuts
    auto *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &EditorTab::saveCurrentFile);

    auto *closeShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this);
    connect(closeShortcut, &QShortcut::activated, this, &EditorTab::closeCurrentTab);

    auto *nextShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), this);
    connect(nextShortcut, &QShortcut::activated, this, &EditorTab::nextTab);
}

void EditorTab::openFile(const QString &filePath)
{
    // Check if already open
    CodeEditor *existing = editorForPath(filePath);
    if (existing) {
        m_subTabs->setCurrentWidget(existing);
        return;
    }

    // Read file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString content = QTextStream(&file).readAll();
    file.close();

    auto *editor = new CodeEditor(m_colors);
    editor->setFilePath(filePath);
    editor->setPlainText(content);
    editor->document()->setModified(false);

    QFileInfo fi(filePath);
    int idx = m_subTabs->addTab(editor, fi.fileName());
    m_subTabs->setCurrentIndex(idx);

    connect(editor, &CodeEditor::modificationChanged, this,
            [this, editor](bool) {
                int i = m_subTabs->indexOf(editor);
                if (i >= 0) updateTabTitle(i);
            });
}

void EditorTab::goToFileAndLine(const QString &filePath, int line)
{
    openFile(filePath);
    CodeEditor *editor = editorForPath(filePath);
    if (editor)
        editor->highlightErrorLine(line);
}

void EditorTab::saveCurrentFile()
{
    auto *editor = currentEditor();
    if (!editor || editor->filePath().isEmpty()) return;

    QFile file(editor->filePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << editor->toPlainText();
        file.close();
        editor->document()->setModified(false);
    }
}

void EditorTab::closeTab(int index)
{
    auto *editor = qobject_cast<CodeEditor*>(m_subTabs->widget(index));
    if (!editor) return;

    if (editor->isModified()) {
        QFileInfo fi(editor->filePath());
        auto result = QMessageBox::question(
            this, "Unsaved Changes",
            QStringLiteral("Save changes to %1?").arg(fi.fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (result == QMessageBox::Cancel) return;
        if (result == QMessageBox::Save) {
            m_subTabs->setCurrentIndex(index);
            saveCurrentFile();
        }
    }
    m_subTabs->removeTab(index);
    delete editor;
}

void EditorTab::closeCurrentTab()
{
    int idx = m_subTabs->currentIndex();
    if (idx >= 0) closeTab(idx);
}

void EditorTab::nextTab()
{
    int count = m_subTabs->count();
    if (count > 1) {
        int next = (m_subTabs->currentIndex() + 1) % count;
        m_subTabs->setCurrentIndex(next);
    }
}

void EditorTab::onTabModified(bool)
{
    // handled by lambda in openFile
}

void EditorTab::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    for (int i = 0; i < m_subTabs->count(); ++i) {
        auto *editor = qobject_cast<CodeEditor*>(m_subTabs->widget(i));
        if (editor) editor->setColors(colors);
    }
}

CodeEditor *EditorTab::editorForPath(const QString &path)
{
    for (int i = 0; i < m_subTabs->count(); ++i) {
        auto *editor = qobject_cast<CodeEditor*>(m_subTabs->widget(i));
        if (editor && editor->filePath() == path)
            return editor;
    }
    return nullptr;
}

CodeEditor *EditorTab::currentEditor()
{
    return qobject_cast<CodeEditor*>(m_subTabs->currentWidget());
}

void EditorTab::updateTabTitle(int index)
{
    auto *editor = qobject_cast<CodeEditor*>(m_subTabs->widget(index));
    if (!editor) return;
    QFileInfo fi(editor->filePath());
    QString title = fi.fileName();
    if (editor->isModified())
        title.prepend(QStringLiteral("\u2022 "));  // bullet dot
    m_subTabs->setTabText(index, title);
}
