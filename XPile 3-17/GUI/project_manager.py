#!/usr/bin/env python3
# GUI/project_manager.py — UniLogic Project Manager
# Desktop GUI (PySide6, dark theme) for managing XPile projects.
# Features: New Project, DR Configurator, Build, DR Conflict Checker.

import sys
import os
import traceback

def _excepthook(exc_type, exc_value, exc_tb):
    error_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "gui_error.log")
    with open(error_path, "w") as f:
        traceback.print_exception(exc_type, exc_value, exc_tb, file=f)

sys.excepthook = _excepthook

import subprocess
import re
import ctypes
from pathlib import Path

from PySide6.QtWidgets import (
    QApplication, QMainWindow, QTabWidget, QWidget, QVBoxLayout, QHBoxLayout,
    QGridLayout, QLabel, QLineEdit, QPushButton, QComboBox, QCheckBox,
    QRadioButton, QButtonGroup, QGroupBox, QFileDialog, QMessageBox,
    QPlainTextEdit, QTreeWidget, QTreeWidgetItem, QHeaderView,
    QTextEdit, QFrame,
)
from PySide6.QtCore import Qt, QThread, Signal
from PySide6.QtGui import QFont, QColor, QIcon

# ── Paths ────────────────────────────────────────────────────────────────

def _resource_path(relative):
    """Resolve path for both dev and PyInstaller --onefile builds."""
    base = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base, relative)

_GUI_DIR  = Path(__file__).resolve().parent
_ROOT_DIR = _GUI_DIR.parent                          # XPile 3-17
_MAIN_PY  = _ROOT_DIR / "Main.py"
_PYTHON   = sys.executable or "python"
_ICON_PATH = _resource_path("GUI/unilogic.ico")

# ── Dark Theme Colors ────────────────────────────────────────────────────

BG_DARK    = "#1e1e1e"
BG_SURFACE = "#252526"
BG_RAISED  = "#2d2d30"
BG_INPUT   = "#333337"
BORDER     = "#3e3e42"
BORDER_LT  = "#4e4e52"
TEXT       = "#d4d4d4"
TEXT_DIM   = "#808080"
TEXT_BRIGHT = "#e8e8e8"
NAVY       = "#1a4a6b"
NAVY_LIGHT = "#2a7ab5"
NAVY_HOVER = "#235d85"
RED        = "#f44747"
GREEN      = "#4ec9b0"
ORANGE     = "#ce9178"
BLUE       = "#569cd6"

# ── DR Settings Specification ────────────────────────────────────────────

DR_SETTINGS = [
    ("memory",      "Memory Model",     ["manual", "gc", "refcount", "arena"],
     "Allocation strategy: manual (free/malloc), gc (garbage collected),\n"
     "refcount (reference counting), arena (region-based)"),

    ("safety",      "Safety Mode",      ["checked", "unchecked"],
     "checked: runtime assertions + killswitch active\n"
     "unchecked: assertions compiled out for max performance"),

    ("types",       "Type System",      ["strict", "dynamic"],
     "strict: all types must be declared explicitly\n"
     "dynamic: type inference and duck typing"),

    ("int_width",   "Integer Width",    ["32", "64", "platform"],
     "Default bit-width for bare 'int' declarations\n"
     "platform: matches target architecture"),

    ("concurrency", "Concurrency",      ["threaded", "parallel", "async", "cooperative"],
     "threaded: OS threads + mutexes\nparallel: fork-join parallel for\n"
     "async: async/await model\ncooperative: coroutine-based"),
]

STANDARD_DIRS = ["Lexer", "Parser", "AST", "Semantic", "Codegen", "REPL", "LSP"]

DR_SETTING_NAMES = [s[0] for s in DR_SETTINGS]


# ── Utility ──────────────────────────────────────────────────────────────

def parse_dr_block(text):
    """Extract DR settings from a UL file's header (supports both old // @dr and new @dr forms)."""
    found = {}
    for m in re.finditer(r"(?://\s*)?@dr\s+(\w+)\s*=\s*(\w+)", text):
        found[m.group(1)] = m.group(2)
    return found


# ── Dark Stylesheet ──────────────────────────────────────────────────────

STYLESHEET = f"""
* {{
    font-family: "Segoe UI";
    font-size: 10pt;
}}

QMainWindow {{
    background-color: {BG_DARK};
}}

QWidget {{
    background-color: {BG_DARK};
    color: {TEXT};
}}

/* ── Tabs ─────────────────────────────────────────────── */

QTabWidget::pane {{
    border: none;
    background-color: {BG_DARK};
    border-top: 1px solid {BORDER};
}}

QTabBar {{
    background-color: {BG_SURFACE};
}}

QTabBar::tab {{
    font-weight: bold;
    padding: 10px 22px;
    color: {TEXT_DIM};
    background: {BG_SURFACE};
    border: none;
    border-bottom: 2px solid transparent;
    margin-right: 1px;
}}

QTabBar::tab:hover {{
    color: {TEXT};
    background: {BG_RAISED};
}}

QTabBar::tab:selected {{
    color: {NAVY_LIGHT};
    background: {BG_DARK};
    border-bottom: 2px solid {NAVY_LIGHT};
}}

/* ── Group Boxes ──────────────────────────────────────── */

QGroupBox {{
    background-color: {BG_SURFACE};
    border: 1px solid {BORDER};
    border-radius: 6px;
    margin-top: 14px;
    padding: 18px 12px 12px 12px;
    font-size: 11pt;
    font-weight: bold;
    color: {NAVY_LIGHT};
}}

QGroupBox::title {{
    subcontrol-origin: margin;
    left: 14px;
    padding: 0 6px;
    background-color: {BG_SURFACE};
}}

/* ── Labels ───────────────────────────────────────────── */

QLabel {{
    background: transparent;
    color: {TEXT};
}}

/* ── Inputs ───────────────────────────────────────────── */

QLineEdit {{
    padding: 5px 8px;
    border: 1px solid {BORDER};
    border-radius: 4px;
    background: {BG_INPUT};
    color: {TEXT_BRIGHT};
    selection-background-color: {NAVY};
}}

QLineEdit:focus {{
    border: 1px solid {NAVY_LIGHT};
}}

QComboBox {{
    padding: 5px 8px;
    border: 1px solid {BORDER};
    border-radius: 4px;
    background: {BG_INPUT};
    color: {TEXT_BRIGHT};
}}

QComboBox:hover {{
    border: 1px solid {BORDER_LT};
}}

QComboBox::drop-down {{
    border: none;
    width: 20px;
}}

QComboBox::down-arrow {{
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid {TEXT_DIM};
    margin-right: 6px;
}}

QComboBox QAbstractItemView {{
    background: {BG_RAISED};
    color: {TEXT};
    border: 1px solid {BORDER};
    selection-background-color: {NAVY};
    selection-color: white;
    outline: none;
}}

/* ── Buttons ──────────────────────────────────────────── */

QPushButton {{
    padding: 6px 16px;
    border: 1px solid {BORDER};
    border-radius: 4px;
    background: {BG_RAISED};
    color: {TEXT};
}}

QPushButton:hover {{
    background: {BORDER_LT};
    border: 1px solid {BORDER_LT};
}}

QPushButton:pressed {{
    background: {BG_INPUT};
}}

/* ── Checkboxes & Radios ──────────────────────────────── */

QCheckBox, QRadioButton {{
    background: transparent;
    color: {TEXT};
    spacing: 6px;
}}

QCheckBox::indicator, QRadioButton::indicator {{
    width: 16px;
    height: 16px;
    border: 1px solid {BORDER_LT};
    border-radius: 3px;
    background: {BG_INPUT};
}}

QRadioButton::indicator {{
    border-radius: 9px;
}}

QCheckBox::indicator:checked {{
    background: {NAVY_LIGHT};
    border-color: {NAVY_LIGHT};
}}

QRadioButton::indicator:checked {{
    background: {NAVY_LIGHT};
    border-color: {NAVY_LIGHT};
}}

/* ── Tree / Table ─────────────────────────────────────── */

QTreeWidget {{
    background: {BG_SURFACE};
    alternate-background-color: {BG_RAISED};
    border: 1px solid {BORDER};
    border-radius: 4px;
    color: {TEXT};
    outline: none;
}}

QTreeWidget::item {{
    padding: 4px 0;
    border-bottom: 1px solid {BORDER};
}}

QTreeWidget::item:selected {{
    background: {NAVY};
    color: white;
}}

QHeaderView {{
    background: {BG_SURFACE};
}}

QHeaderView::section {{
    font-weight: bold;
    background: {BG_RAISED};
    color: {TEXT_DIM};
    border: none;
    border-bottom: 1px solid {BORDER};
    border-right: 1px solid {BORDER};
    padding: 6px 10px;
}}

/* ── Scrollbars ───────────────────────────────────────── */

QScrollBar:vertical {{
    background: {BG_SURFACE};
    width: 10px;
    border: none;
}}

QScrollBar::handle:vertical {{
    background: {BORDER_LT};
    border-radius: 4px;
    min-height: 30px;
}}

QScrollBar::handle:vertical:hover {{
    background: {TEXT_DIM};
}}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
    height: 0;
}}

QScrollBar:horizontal {{
    background: {BG_SURFACE};
    height: 10px;
    border: none;
}}

QScrollBar::handle:horizontal {{
    background: {BORDER_LT};
    border-radius: 4px;
    min-width: 30px;
}}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {{
    width: 0;
}}

/* ── PlainTextEdit ────────────────────────────────────── */

QPlainTextEdit {{
    background: {BG_INPUT};
    color: {TEXT};
    border: 1px solid {BORDER};
    border-radius: 4px;
    selection-background-color: {NAVY};
}}

/* ── TextEdit (build log) ─────────────────────────────── */

QTextEdit {{
    background: #111114;
    color: {TEXT};
    border: 1px solid {BORDER};
    border-radius: 4px;
}}

/* ── Message Boxes ────────────────────────────────────── */

QMessageBox {{
    background: {BG_SURFACE};
}}

QMessageBox QLabel {{
    color: {TEXT};
}}

QMessageBox QPushButton {{
    min-width: 80px;
}}
"""

NAVY_BTN = f"""
QPushButton {{
    background-color: {NAVY};
    color: white;
    font-weight: bold;
    border: none;
    border-radius: 5px;
    padding: 7px 24px;
}}
QPushButton:hover {{
    background-color: {NAVY_HOVER};
}}
QPushButton:pressed {{
    background-color: #163d58;
}}
QPushButton:disabled {{
    background-color: {BG_RAISED};
    color: {TEXT_DIM};
}}
"""


# ═════════════════════════════════════════════════════════════════════════
# New Project Tab
# ═════════════════════════════════════════════════════════════════════════

class NewProjectTab(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)

        # ── Header ───────────────────────────────────────────────────────
        title = QLabel("New UniLogic Project")
        title.setStyleSheet(f"font-size: 14pt; font-weight: bold; color: {NAVY_LIGHT};")
        layout.addWidget(title)

        subtitle = QLabel("Creates the standard XPile folder structure with a blank entry point.")
        subtitle.setStyleSheet(f"color: {TEXT_DIM};")
        layout.addWidget(subtitle)

        # ── Form ─────────────────────────────────────────────────────────
        form_group = QGroupBox("Project Settings")
        form_layout = QGridLayout(form_group)
        form_layout.setColumnStretch(1, 1)
        form_layout.setVerticalSpacing(10)
        form_layout.setHorizontalSpacing(12)

        # Project Name
        lbl = QLabel("Project Name:")
        lbl.setStyleSheet("font-weight: bold;")
        form_layout.addWidget(lbl, 0, 0)
        self.name_edit = QLineEdit("MyProject")
        form_layout.addWidget(self.name_edit, 0, 1)

        # Location
        lbl = QLabel("Location:")
        lbl.setStyleSheet("font-weight: bold;")
        form_layout.addWidget(lbl, 1, 0)
        loc_row = QHBoxLayout()
        self.loc_edit = QLineEdit(str(Path.home() / "UniLogic"))
        loc_row.addWidget(self.loc_edit)
        browse_btn = QPushButton("Browse...")
        browse_btn.clicked.connect(self._browse)
        loc_row.addWidget(browse_btn)
        form_layout.addLayout(loc_row, 1, 1)

        # Entry File
        lbl = QLabel("Entry File:")
        lbl.setStyleSheet("font-weight: bold;")
        form_layout.addWidget(lbl, 2, 0)
        self.entry_edit = QLineEdit("main.ul")
        form_layout.addWidget(self.entry_edit, 2, 1)

        # DR Preset
        lbl = QLabel("DR Preset:")
        lbl.setStyleSheet("font-weight: bold;")
        form_layout.addWidget(lbl, 3, 0)
        self.preset_combo = QComboBox()
        self.preset_combo.addItems(["(none)", "Safe Default", "High Performance", "Embedded"])
        form_layout.addWidget(self.preset_combo, 3, 1)

        layout.addWidget(form_group)

        # ── Directories ──────────────────────────────────────────────────
        dirs_group = QGroupBox("Directories to Create")
        dirs_layout = QGridLayout(dirs_group)
        dirs_layout.setVerticalSpacing(8)
        self.dir_checks = {}
        for i, d in enumerate(STANDARD_DIRS):
            cb = QCheckBox(d)
            cb.setChecked(True)
            self.dir_checks[d] = cb
            dirs_layout.addWidget(cb, i // 4, i % 4)
        layout.addWidget(dirs_group)

        # ── Create button ────────────────────────────────────────────────
        btn_row = QHBoxLayout()
        self.create_btn = QPushButton("Create Project")
        self.create_btn.setStyleSheet(NAVY_BTN)
        self.create_btn.clicked.connect(self._create)
        btn_row.addWidget(self.create_btn)
        self.status_lbl = QLabel("")
        btn_row.addWidget(self.status_lbl)
        btn_row.addStretch()
        layout.addLayout(btn_row)
        layout.addStretch()

    def _browse(self):
        path = QFileDialog.getExistingDirectory(self, "Select parent folder")
        if path:
            self.loc_edit.setText(path)

    def _create(self):
        name = self.name_edit.text().strip()
        loc = self.loc_edit.text().strip()
        entry = self.entry_edit.text().strip()

        if not name:
            QMessageBox.warning(self, "Missing Name", "Enter a project name.")
            return
        if not loc:
            QMessageBox.warning(self, "Missing Location", "Select a folder location.")
            return
        if not entry.endswith(".ul"):
            entry += ".ul"

        project_dir = Path(loc) / name
        try:
            project_dir.mkdir(parents=True, exist_ok=True)

            for d, cb in self.dir_checks.items():
                if cb.isChecked():
                    (project_dir / d).mkdir(exist_ok=True)

            entry_path = project_dir / entry
            content = self._make_entry_content(name)
            entry_path.write_text(content, encoding="utf-8")

            self.status_lbl.setText(f"Created: {project_dir}")
            self.status_lbl.setStyleSheet(f"color: {GREEN};")
        except Exception as e:
            self.status_lbl.setText(f"Error: {e}")
            self.status_lbl.setStyleSheet(f"color: {RED};")

    def _make_entry_content(self, name):
        lines = [f"// {name} — UniLogic project entry point\n"]

        preset = self.preset_combo.currentText()
        dr = None
        if preset == "Safe Default":
            dr = {"memory": "gc", "safety": "checked", "types": "strict",
                  "int_width": "64", "concurrency": "threaded"}
        elif preset == "High Performance":
            dr = {"memory": "manual", "safety": "unchecked", "types": "strict",
                  "int_width": "64", "concurrency": "parallel"}
        elif preset == "Embedded":
            dr = {"memory": "arena", "safety": "checked", "types": "strict",
                  "int_width": "32", "concurrency": "cooperative"}

        if dr:
            lines.append("// DR Configuration")
            for key, val in dr.items():
                lines.append(f"@dr {key} = {val}")
            lines.append("")

        lines.append("function main() returns int")
        lines.append("  print 0")
        lines.append("  return 0")
        lines.append("end function")
        lines.append("")
        return "\n".join(lines)


# ═════════════════════════════════════════════════════════════════════════
# DR Configurator Tab
# ═════════════════════════════════════════════════════════════════════════

class DRConfiguratorTab(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)

        # ── Header ───────────────────────────────────────────────────────
        title = QLabel("DR Configurator")
        title.setStyleSheet(f"font-size: 14pt; font-weight: bold; color: {NAVY_LIGHT};")
        layout.addWidget(title)

        subtitle = QLabel("Configure Dynamic Runtime settings and write the DR header block into a .ul file.")
        subtitle.setStyleSheet(f"color: {TEXT_DIM};")
        layout.addWidget(subtitle)

        # ── File selector ────────────────────────────────────────────────
        file_row = QHBoxLayout()
        lbl = QLabel("Target File:")
        lbl.setStyleSheet("font-weight: bold;")
        file_row.addWidget(lbl)
        self.file_edit = QLineEdit()
        file_row.addWidget(self.file_edit, 1)
        browse_btn = QPushButton("Browse...")
        browse_btn.clicked.connect(self._browse_file)
        file_row.addWidget(browse_btn)
        load_btn = QPushButton("Load")
        load_btn.clicked.connect(self._load_file)
        file_row.addWidget(load_btn)
        layout.addLayout(file_row)

        # ── Settings grid ────────────────────────────────────────────────
        settings_group = QGroupBox("DR Settings")
        settings_layout = QGridLayout(settings_group)
        settings_layout.setColumnStretch(3, 1)
        settings_layout.setVerticalSpacing(10)
        settings_layout.setHorizontalSpacing(10)

        self.setting_combos = {}
        self.enable_checks = {}

        for i, (key, label, options, desc) in enumerate(DR_SETTINGS):
            cb = QCheckBox()
            cb.setChecked(False)
            self.enable_checks[key] = cb
            settings_layout.addWidget(cb, i, 0)

            lbl = QLabel(label)
            lbl.setStyleSheet("font-weight: bold;")
            lbl.setFixedWidth(130)
            settings_layout.addWidget(lbl, i, 1)

            combo = QComboBox()
            combo.addItems(options)
            combo.setFixedWidth(150)
            self.setting_combos[key] = combo
            settings_layout.addWidget(combo, i, 2)

            desc_lbl = QLabel(desc.split("\n")[0])
            desc_lbl.setStyleSheet(f"color: {TEXT_DIM}; font-size: 9pt;")
            settings_layout.addWidget(desc_lbl, i, 3)

            cb.stateChanged.connect(self._update_preview)
            combo.currentIndexChanged.connect(self._update_preview)

        layout.addWidget(settings_group)

        # ── Preview ──────────────────────────────────────────────────────
        prev_lbl = QLabel("Preview:")
        prev_lbl.setStyleSheet("font-weight: bold;")
        layout.addWidget(prev_lbl)

        self.preview = QPlainTextEdit()
        self.preview.setReadOnly(True)
        self.preview.setFont(QFont("Consolas", 10))
        self.preview.setMaximumHeight(140)
        layout.addWidget(self.preview)
        self._update_preview()

        # ── Write button ─────────────────────────────────────────────────
        btn_row = QHBoxLayout()
        write_btn = QPushButton("Write to File")
        write_btn.setStyleSheet(NAVY_BTN)
        write_btn.clicked.connect(self._write)
        btn_row.addWidget(write_btn)
        self.write_status = QLabel("")
        btn_row.addWidget(self.write_status)
        btn_row.addStretch()
        layout.addLayout(btn_row)
        layout.addStretch()

    def _browse_file(self):
        path, _ = QFileDialog.getOpenFileName(
            self, "Select .ul file", "", "UniLogic (*.ul);;All Files (*)")
        if path:
            self.file_edit.setText(path)

    def _load_file(self):
        path = self.file_edit.text().strip()
        if not path or not os.path.isfile(path):
            QMessageBox.warning(self, "File Error", "Select a valid .ul file.")
            return

        text = Path(path).read_text(encoding="utf-8")
        found = parse_dr_block(text)

        for key in self.enable_checks:
            self.enable_checks[key].setChecked(False)

        for key, val in found.items():
            if key in self.enable_checks:
                self.enable_checks[key].setChecked(True)
                idx = self.setting_combos[key].findText(val)
                if idx >= 0:
                    self.setting_combos[key].setCurrentIndex(idx)

        count = len(found)
        self.write_status.setText(
            f"Loaded {count} DR setting{'s' if count != 1 else ''} from file.")
        self.write_status.setStyleSheet(f"color: {NAVY_LIGHT};")

    def _get_dr_lines(self):
        lines = []
        for key, label, options, desc in DR_SETTINGS:
            if self.enable_checks[key].isChecked():
                val = self.setting_combos[key].currentText()
                lines.append(f"@dr {key} = {val}")
        return lines

    def _update_preview(self):
        lines = self._get_dr_lines()
        if lines:
            text = "// DR Configuration\n" + "\n".join(lines)
        else:
            text = "(no DR settings enabled)"
        self.preview.setPlainText(text)

    def _write(self):
        path = self.file_edit.text().strip()
        if not path:
            QMessageBox.warning(self, "No File", "Select a target .ul file first.")
            return

        dr_lines = self._get_dr_lines()
        if not dr_lines:
            QMessageBox.information(self, "Nothing to Write", "Enable at least one DR setting.")
            return

        try:
            p = Path(path)
            existing = p.read_text(encoding="utf-8") if p.exists() else ""

            cleaned = re.sub(
                r"(//\s*DR Configuration\n)?((?://\s*)?@dr\s+\w+\s*=\s*\w+\n)*",
                "", existing).lstrip("\n")

            header = "// DR Configuration\n" + "\n".join(dr_lines) + "\n\n"
            p.write_text(header + cleaned, encoding="utf-8")

            self.write_status.setText(f"Wrote {len(dr_lines)} settings to {p.name}")
            self.write_status.setStyleSheet(f"color: {GREEN};")
        except Exception as e:
            self.write_status.setText(f"Error: {e}")
            self.write_status.setStyleSheet(f"color: {RED};")


# ═════════════════════════════════════════════════════════════════════════
# Build Tab
# ═════════════════════════════════════════════════════════════════════════

class BuildWorker(QThread):
    """Runs the XPile compiler in a background thread."""
    log_line = Signal(str, str)
    finished_build = Signal(int)

    def __init__(self, src, target, out):
        super().__init__()
        self.src = src
        self.target = target
        self.out = out

    def run(self):
        cmd = [_PYTHON, str(_MAIN_PY), self.src, "-t", self.target]
        if self.out and self.out != "(auto)":
            cmd += ["-o", self.out]

        try:
            result = subprocess.run(
                cmd, capture_output=True, text=True, timeout=60,
                cwd=str(Path(self.src).parent))

            if result.stdout:
                for line in result.stdout.strip().splitlines():
                    tag = "success" if "compiled" in line.lower() else ""
                    self.log_line.emit(line, tag)
            if result.stderr:
                for line in result.stderr.strip().splitlines():
                    self.log_line.emit(line, "error")

            self.finished_build.emit(result.returncode)

        except subprocess.TimeoutExpired:
            self.log_line.emit("[timeout] Build exceeded 60 seconds.", "error")
            self.finished_build.emit(-1)
        except Exception as e:
            self.log_line.emit(f"[error] {e}", "error")
            self.finished_build.emit(-1)


class BuildTab(QWidget):
    def __init__(self):
        super().__init__()
        self._worker = None
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)

        # ── Header ───────────────────────────────────────────────────────
        title = QLabel("Build")
        title.setStyleSheet(f"font-size: 14pt; font-weight: bold; color: {NAVY_LIGHT};")
        layout.addWidget(title)

        subtitle = QLabel("Compile a .ul file using the XPile pipeline.")
        subtitle.setStyleSheet(f"color: {TEXT_DIM};")
        layout.addWidget(subtitle)

        # ── Build config ─────────────────────────────────────────────────
        cfg_group = QGroupBox("Build Configuration")
        cfg_layout = QGridLayout(cfg_group)
        cfg_layout.setColumnStretch(1, 1)
        cfg_layout.setVerticalSpacing(10)
        cfg_layout.setHorizontalSpacing(12)

        # Source File
        lbl = QLabel("Source File:")
        lbl.setStyleSheet("font-weight: bold;")
        cfg_layout.addWidget(lbl, 0, 0)
        src_row = QHBoxLayout()
        self.src_edit = QLineEdit()
        src_row.addWidget(self.src_edit)
        browse_btn = QPushButton("Browse...")
        browse_btn.clicked.connect(self._browse)
        src_row.addWidget(browse_btn)
        cfg_layout.addLayout(src_row, 0, 1)

        # Target
        lbl = QLabel("Target:")
        lbl.setStyleSheet("font-weight: bold;")
        cfg_layout.addWidget(lbl, 1, 0)
        target_row = QHBoxLayout()
        self.target_group = QButtonGroup(self)
        self._target_value = "c"
        for val, label in [("c", "C"), ("llvm", "LLVM IR"),
                           ("python", "Python"), ("js", "JavaScript")]:
            rb = QRadioButton(label)
            rb.setChecked(val == "c")
            rb.toggled.connect(lambda checked, v=val: self._set_target(v) if checked else None)
            self.target_group.addButton(rb)
            target_row.addWidget(rb)
        target_row.addStretch()
        cfg_layout.addLayout(target_row, 1, 1)

        # Output
        lbl = QLabel("Output:")
        lbl.setStyleSheet("font-weight: bold;")
        cfg_layout.addWidget(lbl, 2, 0)
        self.out_edit = QLineEdit("(auto)")
        cfg_layout.addWidget(self.out_edit, 2, 1)

        layout.addWidget(cfg_group)

        # ── Build button row ─────────────────────────────────────────────
        btn_row = QHBoxLayout()
        self.build_btn = QPushButton("Build")
        self.build_btn.setStyleSheet(NAVY_BTN)
        self.build_btn.clicked.connect(self._build)
        btn_row.addWidget(self.build_btn)
        self.status_lbl = QLabel("")
        btn_row.addWidget(self.status_lbl)
        btn_row.addStretch()
        layout.addLayout(btn_row)

        # ── Log panel ────────────────────────────────────────────────────
        log_group = QGroupBox("Compiler Output")
        log_layout = QVBoxLayout(log_group)

        self.log = QTextEdit()
        self.log.setReadOnly(True)
        self.log.setFont(QFont("Consolas", 10))
        self.log.setMinimumHeight(200)
        log_layout.addWidget(self.log)

        layout.addWidget(log_group, 1)

    def _set_target(self, val):
        self._target_value = val

    def _browse(self):
        path, _ = QFileDialog.getOpenFileName(
            self, "Select .ul file", "", "UniLogic (*.ul);;All Files (*)")
        if path:
            self.src_edit.setText(path)

    def _log_line(self, text, tag=""):
        if tag == "error":
            color = RED
        elif tag == "success":
            color = GREEN
        elif tag == "info":
            color = BLUE
        else:
            color = TEXT
        self.log.append(f'<span style="color:{color}; font-family:Consolas;">{text}</span>')

    def _build(self):
        src = self.src_edit.text().strip()
        if not src or not os.path.isfile(src):
            QMessageBox.warning(self, "No Source", "Select a valid .ul source file.")
            return

        target = self._target_value
        out = self.out_edit.text().strip()

        self.log.clear()
        self.build_btn.setEnabled(False)
        self.status_lbl.setText("Building...")
        self.status_lbl.setStyleSheet(f"color: {ORANGE};")
        self._log_line(f"[build] {os.path.basename(src)}  target={target}", "info")

        self._worker = BuildWorker(src, target, out)
        self._worker.log_line.connect(self._log_line)
        self._worker.finished_build.connect(self._build_done)
        self._worker.start()

    def _build_done(self, returncode):
        self.build_btn.setEnabled(True)
        if returncode == 0:
            self.status_lbl.setText("Build succeeded")
            self.status_lbl.setStyleSheet(f"color: {GREEN};")
        else:
            self.status_lbl.setText(f"Build failed (exit {returncode})")
            self.status_lbl.setStyleSheet(f"color: {RED};")


# ═════════════════════════════════════════════════════════════════════════
# DR Conflict Checker Tab
# ═════════════════════════════════════════════════════════════════════════

class DRConflictTab(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)

        # ── Header ───────────────────────────────────────────────────────
        title = QLabel("DR Conflict Checker")
        title.setStyleSheet(f"font-size: 14pt; font-weight: bold; color: {NAVY_LIGHT};")
        layout.addWidget(title)

        subtitle = QLabel("Scan all .ul files in a project for conflicting DR settings.")
        subtitle.setStyleSheet(f"color: {TEXT_DIM};")
        layout.addWidget(subtitle)

        # ── Folder selector ──────────────────────────────────────────────
        folder_row = QHBoxLayout()
        lbl = QLabel("Project Folder:")
        lbl.setStyleSheet("font-weight: bold;")
        folder_row.addWidget(lbl)
        self.folder_edit = QLineEdit()
        folder_row.addWidget(self.folder_edit, 1)
        browse_btn = QPushButton("Browse...")
        browse_btn.clicked.connect(self._browse)
        folder_row.addWidget(browse_btn)
        scan_btn = QPushButton("Scan")
        scan_btn.setStyleSheet(NAVY_BTN)
        scan_btn.clicked.connect(self._scan)
        folder_row.addWidget(scan_btn)
        layout.addLayout(folder_row)

        # ── Results table ────────────────────────────────────────────────
        results_group = QGroupBox("Results")
        results_layout = QVBoxLayout(results_group)

        self.tree = QTreeWidget()
        headers = ["File"] + [k.replace("_", " ").title() for k in DR_SETTING_NAMES]
        self.tree.setHeaderLabels(headers)
        self.tree.setAlternatingRowColors(True)
        self.tree.setRootIsDecorated(False)
        header = self.tree.header()
        header.setStretchLastSection(True)
        header.setSectionResizeMode(0, QHeaderView.ResizeMode.Stretch)
        for i in range(1, len(headers)):
            header.setSectionResizeMode(i, QHeaderView.ResizeMode.ResizeToContents)
        results_layout.addWidget(self.tree)

        layout.addWidget(results_group, 1)

        # ── Summary ──────────────────────────────────────────────────────
        self.summary_lbl = QLabel("")
        self.summary_lbl.setWordWrap(True)
        layout.addWidget(self.summary_lbl)

    def _browse(self):
        path = QFileDialog.getExistingDirectory(self, "Select project folder")
        if path:
            self.folder_edit.setText(path)

    def _scan(self):
        folder = self.folder_edit.text().strip()
        if not folder or not os.path.isdir(folder):
            QMessageBox.warning(self, "Invalid Folder", "Select a valid project folder.")
            return

        self.tree.clear()

        ul_files = sorted(Path(folder).rglob("*.ul"))
        if not ul_files:
            self.summary_lbl.setText("No .ul files found in the selected folder.")
            self.summary_lbl.setStyleSheet(f"color: {ORANGE};")
            return

        all_settings = {}
        value_sets = {key: set() for key in DR_SETTING_NAMES}

        for f in ul_files:
            rel = str(f.relative_to(folder))
            text = f.read_text(encoding="utf-8", errors="replace")
            dr = parse_dr_block(text)
            all_settings[rel] = dr
            for key, val in dr.items():
                if key in value_sets:
                    value_sets[key].add(val)

        conflicts = {key for key, vals in value_sets.items() if len(vals) > 1}

        for rel, dr in all_settings.items():
            values = [rel]
            has_conflict = False
            for key in DR_SETTING_NAMES:
                val = dr.get(key, "\u2014")
                values.append(val)
                if key in conflicts and val != "\u2014":
                    has_conflict = True

            item = QTreeWidgetItem(values)
            if has_conflict:
                for col in range(len(values)):
                    item.setForeground(col, QColor(RED))
            self.tree.addTopLevelItem(item)

        n_files = len(ul_files)
        n_with_dr = sum(1 for dr in all_settings.values() if dr)
        if conflicts:
            clist = ", ".join(sorted(conflicts))
            self.summary_lbl.setText(
                f"Scanned {n_files} files ({n_with_dr} with DR headers). "
                f"CONFLICTS in: {clist}")
            self.summary_lbl.setStyleSheet(f"color: {RED};")
        else:
            self.summary_lbl.setText(
                f"Scanned {n_files} files ({n_with_dr} with DR headers). "
                f"No conflicts found.")
            self.summary_lbl.setStyleSheet(f"color: {GREEN};")


# ═════════════════════════════════════════════════════════════════════════
# Main Application Window
# ═════════════════════════════════════════════════════════════════════════

class ProjectManager(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("UniLogic Project Manager")
        self.resize(820, 680)
        self.setMinimumSize(700, 500)
        self.setWindowIcon(QIcon(_ICON_PATH))

        central = QWidget()
        self.setCentralWidget(central)
        main_layout = QVBoxLayout(central)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        # ── Header bar ───────────────────────────────────────────────────
        header_bar = QFrame()
        header_bar.setFixedHeight(48)
        header_bar.setStyleSheet(f"""
            QFrame {{
                background-color: {NAVY};
                border-bottom: 1px solid #163d58;
            }}
        """)
        hb_layout = QHBoxLayout(header_bar)
        hb_layout.setContentsMargins(20, 0, 20, 0)

        title_lbl = QLabel(
            '<span style="font-size:28px; vertical-align:middle;">\u221e</span>'
            '<span style="font-size:20px; vertical-align:middle;"> UniLogic Project Manager</span>')
        title_lbl.setStyleSheet("color: white; font-weight: bold; background: transparent;")
        title_lbl.setAlignment(Qt.AlignVCenter)
        hb_layout.addWidget(title_lbl)

        hb_layout.addStretch()

        version_lbl = QLabel("XPile 3-17")
        version_lbl.setStyleSheet(f"""
            color: #7fb3d4;
            font-size: 10pt;
            background: transparent;
        """)
        hb_layout.addWidget(version_lbl)

        main_layout.addWidget(header_bar)

        # ── Tab widget ───────────────────────────────────────────────────
        self.tabs = QTabWidget()
        self.tabs.addTab(NewProjectTab(),     "  New Project  ")
        self.tabs.addTab(DRConfiguratorTab(), "  DR Config  ")
        self.tabs.addTab(BuildTab(),          "  Build  ")
        self.tabs.addTab(DRConflictTab(),     "  DR Conflicts  ")

        main_layout.addWidget(self.tabs)


# ═════════════════════════════════════════════════════════════════════════

def main():
    ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(
        "UniLogic.ProjectManager")
    app = QApplication(sys.argv)
    app.setStyleSheet(STYLESHEET)
    window = ProjectManager()

    # Always start centered on screen, never restore off-screen position
    screen = QApplication.primaryScreen().geometry()
    window.resize(1000, 700)
    window.move(
        (screen.width() - 1000) // 2,
        (screen.height() - 700) // 2
    )

    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
