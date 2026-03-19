#pragma once
// editortab.h -- Code editor tab with multi-file support and syntax highlighting

#include <QWidget>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTimer>
#include "theme.h"

// ── UniLogic syntax highlighter ─────────────────────────────────────────

class ULHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit ULHighlighter(const ThemeColors &colors, QTextDocument *parent = nullptr);
    void updateColors(const ThemeColors &colors);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    QVector<Rule> m_rules;
    void buildRules(const ThemeColors &c);
};

// ── Line number area ────────────────────────────────────────────────────

class CodeEditor;

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *m_editor;
};

// ── Code editor widget ──────────────────────────────────────────────────

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(const ThemeColors &colors, QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setColors(const ThemeColors &colors);
    void goToLine(int line);

    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &path) { m_filePath = path; }
    bool isModified() const { return document()->isModified(); }

    void highlightErrorLine(int line);

signals:
    void modificationChanged(bool modified);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void clearErrorHighlight();

private:
    LineNumberArea  *m_lineNumberArea;
    ULHighlighter   *m_highlighter;
    ThemeColors      m_colors;
    QString          m_filePath;
    QTimer           m_errorTimer;
    int              m_errorLine = -1;
};

// ── Multi-file editor tab ───────────────────────────────────────────────

class EditorTab : public QWidget
{
    Q_OBJECT
public:
    explicit EditorTab(const ThemeColors &colors, QWidget *parent = nullptr);

    void openFile(const QString &filePath);
    void goToFileAndLine(const QString &filePath, int line);

public slots:
    void onThemeChanged(ThemeMode mode, const ThemeColors &colors);

private slots:
    void saveCurrentFile();
    void closeTab(int index);
    void onTabModified(bool modified);
    void closeCurrentTab();
    void nextTab();

private:
    CodeEditor *editorForPath(const QString &path);
    CodeEditor *currentEditor();
    void updateTabTitle(int index);

    ThemeColors m_colors;
    QTabWidget *m_subTabs = nullptr;
};
