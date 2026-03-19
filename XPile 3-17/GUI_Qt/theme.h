#pragma once
// theme.h -- Dark/Light theme colors and stylesheet generation
// for the UniLogic Project Manager (Qt C++ port).

#include <QString>

// ── Theme mode ──────────────────────────────────────────────────────────

enum class ThemeMode { Dark, Light, HighContrast, SolarizedDark, Monokai };

// ── Color palette ───────────────────────────────────────────────────────

struct ThemeColors {
    QString bgDark;
    QString bgSurface;
    QString bgRaised;
    QString bgInput;
    QString border;
    QString borderLt;
    QString text;
    QString textDim;
    QString textBright;
    QString navy;
    QString navyLight;
    QString navyHover;
    QString red;
    QString green;
    QString orange;
    QString blue;
    QString logBg;
    QString headerBorderBottom;
    QString versionColor;
};

inline ThemeColors darkColors()
{
    return {
        "#1e1e1e",   // bgDark
        "#252526",   // bgSurface
        "#2d2d30",   // bgRaised
        "#333337",   // bgInput
        "#3e3e42",   // border
        "#4e4e52",   // borderLt
        "#d4d4d4",   // text
        "#808080",   // textDim
        "#e8e8e8",   // textBright
        "#1a4a6b",   // navy  (primary accent)
        "#2a7ab5",   // navyLight
        "#235d85",   // navyHover
        "#f44747",   // red
        "#4ec9b0",   // green
        "#ce9178",   // orange
        "#569cd6",   // blue
        "#111114",   // logBg
        "#163d58",   // headerBorderBottom
        "#7fb3d4",   // versionColor
    };
}

inline ThemeColors lightColors()
{
    return {
        "#f0f0f2",   // bgDark  (light background)
        "#ffffff",   // bgSurface
        "#e6e6ea",   // bgRaised
        "#ffffff",   // bgInput
        "#c4c4c8",   // border
        "#a8a8ac",   // borderLt
        "#1e1e1e",   // text
        "#666666",   // textDim
        "#000000",   // textBright
        "#1a4a6b",   // navy  (same accent)
        "#1a6daa",   // navyLight (slightly darker for contrast on white)
        "#235d85",   // navyHover
        "#c62828",   // red
        "#1b7a5a",   // green
        "#a0522d",   // orange
        "#1565c0",   // blue
        "#f5f5f7",   // logBg
        "#0f3048",   // headerBorderBottom
        "#7fb3d4",   // versionColor
    };
}

inline ThemeColors highContrastColors()
{
    return {
        "#000000", "#0a0a0a", "#1a1a1a", "#0a0a0a",
        "#ffffff", "#ffffff",
        "#ffffff", "#cccccc", "#ffffff",
        "#1a4a6b", "#ffff00", "#ffcc00",
        "#ff4444", "#00ff00", "#ffaa00", "#00ccff",
        "#000000", "#ffff00", "#ffff00",
    };
}

inline ThemeColors solarizedDarkColors()
{
    return {
        "#002b36", "#073642", "#073642", "#002b36",
        "#586e75", "#657b83",
        "#839496", "#586e75", "#93a1a1",
        "#268bd2", "#268bd2", "#1a6fb5",
        "#dc322f", "#2aa198", "#cb4b16", "#268bd2",
        "#001e26", "#073642", "#586e75",
    };
}

inline ThemeColors monokaiColors()
{
    return {
        "#272822", "#2e2e28", "#3e3d32", "#3e3d32",
        "#49483e", "#75715e",
        "#f8f8f2", "#75715e", "#f8f8f2",
        "#66d9ef", "#66d9ef", "#529db5",
        "#f92672", "#a6e22e", "#e6db74", "#66d9ef",
        "#1e1e19", "#49483e", "#75715e",
    };
}

inline ThemeColors colorsForThemeName(const QString &name)
{
    if (name == "Light")          return lightColors();
    if (name == "High Contrast")  return highContrastColors();
    if (name == "Solarized Dark") return solarizedDarkColors();
    if (name == "Monokai")        return monokaiColors();
    return darkColors();  // "Dark Navy" or default
}

// ── Stylesheet generator ────────────────────────────────────────────────

inline QString buildStylesheet(const ThemeColors& c)
{
    return QStringLiteral(R"SS(
* {
    font-family: "Segoe UI";
    font-size: 10pt;
}

QMainWindow {
    background-color: %1;
}

QWidget {
    background-color: %1;
    color: %7;
}

/* ── Tabs ─────────────────────────────────────────────── */

QTabWidget::pane {
    border: none;
    background-color: %1;
    border-top: 1px solid %5;
}

QTabBar {
    background-color: %2;
}

QTabBar::tab {
    font-weight: bold;
    padding: 10px 22px;
    color: %8;
    background: %2;
    border: none;
    border-bottom: 2px solid transparent;
    margin-right: 1px;
}

QTabBar::tab:hover {
    color: %7;
    background: %3;
}

QTabBar::tab:selected {
    color: %11;
    background: %1;
    border-bottom: 2px solid %11;
}

/* ── Group Boxes ──────────────────────────────────────── */

QGroupBox {
    background-color: %2;
    border: 1px solid %5;
    border-radius: 6px;
    margin-top: 14px;
    padding: 18px 12px 12px 12px;
    font-size: 11pt;
    font-weight: bold;
    color: %11;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 14px;
    padding: 0 6px;
    background-color: %2;
}

/* ── Labels ───────────────────────────────────────────── */

QLabel {
    background: transparent;
    color: %7;
}

/* ── Inputs ───────────────────────────────────────────── */

QLineEdit {
    padding: 5px 8px;
    border: 1px solid %5;
    border-radius: 4px;
    background: %4;
    color: %9;
    selection-background-color: %10;
}

QLineEdit:focus {
    border: 1px solid %11;
}

QComboBox {
    padding: 5px 8px;
    border: 1px solid %5;
    border-radius: 4px;
    background: %4;
    color: %9;
}

QComboBox:hover {
    border: 1px solid %6;
}

QComboBox::drop-down {
    border: none;
    width: 20px;
}

QComboBox::down-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid %8;
    margin-right: 6px;
}

QComboBox QAbstractItemView {
    background: %3;
    color: %7;
    border: 1px solid %5;
    selection-background-color: %10;
    selection-color: white;
    outline: none;
}

/* ── Buttons ──────────────────────────────────────────── */

QPushButton {
    padding: 6px 16px;
    border: 1px solid %5;
    border-radius: 4px;
    background: %3;
    color: %7;
}

QPushButton:hover {
    background: %6;
    border: 1px solid %6;
}

QPushButton:pressed {
    background: %4;
}

/* ── Checkboxes & Radios ──────────────────────────────── */

QCheckBox, QRadioButton {
    background: transparent;
    color: %7;
    spacing: 6px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid %6;
    border-radius: 3px;
    background: %4;
}

QRadioButton::indicator {
    border-radius: 9px;
}

QCheckBox::indicator:checked {
    background: %11;
    border-color: %11;
}

QRadioButton::indicator:checked {
    background: %11;
    border-color: %11;
}

/* ── Tree / Table ─────────────────────────────────────── */

QTreeWidget {
    background: %2;
    alternate-background-color: %3;
    border: 1px solid %5;
    border-radius: 4px;
    color: %7;
    outline: none;
}

QTreeWidget::item {
    padding: 4px 0;
    border-bottom: 1px solid %5;
}

QTreeWidget::item:selected {
    background: %10;
    color: white;
}

QHeaderView {
    background: %2;
}

QHeaderView::section {
    font-weight: bold;
    background: %3;
    color: %8;
    border: none;
    border-bottom: 1px solid %5;
    border-right: 1px solid %5;
    padding: 6px 10px;
}

/* ── Scrollbars ───────────────────────────────────────── */

QScrollBar:vertical {
    background: %2;
    width: 10px;
    border: none;
}

QScrollBar::handle:vertical {
    background: %6;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: %8;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background: %2;
    height: 10px;
    border: none;
}

QScrollBar::handle:horizontal {
    background: %6;
    border-radius: 4px;
    min-width: 30px;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

/* ── PlainTextEdit ────────────────────────────────────── */

QPlainTextEdit {
    background: %4;
    color: %7;
    border: 1px solid %5;
    border-radius: 4px;
    selection-background-color: %10;
}

/* ── TextEdit (build log) ─────────────────────────────── */

QTextEdit {
    background: %17;
    color: %7;
    border: 1px solid %5;
    border-radius: 4px;
}

/* ── Message Boxes ────────────────────────────────────── */

QMessageBox {
    background: %2;
}

QMessageBox QLabel {
    color: %7;
}

QMessageBox QPushButton {
    min-width: 80px;
}
)SS")
        .arg(c.bgDark,      // %1
             c.bgSurface,    // %2
             c.bgRaised,     // %3
             c.bgInput,      // %4
             c.border,       // %5
             c.borderLt,     // %6
             c.text,         // %7
             c.textDim,      // %8
             c.textBright)   // %9
        .arg(c.navy,         // %10
             c.navyLight,    // %11
             c.navyHover,    // %12
             c.red,          // %13
             c.green,        // %14
             c.orange,       // %15
             c.blue,         // %16
             c.logBg);       // %17
}

inline QString navyBtnStyle(const ThemeColors& c)
{
    return QStringLiteral(
        "QPushButton { background-color: %1; color: white; font-weight: bold;"
        " border: none; border-radius: 5px; padding: 7px 24px; }"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:pressed { background-color: #163d58; }"
        "QPushButton:disabled { background-color: %3; color: %4; }")
        .arg(c.navy, c.navyHover, c.bgRaised, c.textDim);
}

inline QString headerBarStyle(const ThemeColors& c)
{
    return QStringLiteral(
        "QFrame { background-color: %1; border-bottom: 1px solid %2; }")
        .arg(c.navy, c.headerBorderBottom);
}

inline QString themeBtnStyle()
{
    return QStringLiteral(
        "QPushButton { background: transparent; border: none;"
        " color: white; font-size: 14pt; padding: 4px 8px; }"
        "QPushButton:hover { background: rgba(255,255,255,40); border-radius: 4px; }");
}
