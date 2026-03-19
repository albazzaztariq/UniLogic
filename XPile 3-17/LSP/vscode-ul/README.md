# UniLogic Language Extension for VS Code

Language support for UniLogic (`.ul`) files — the language compiled by XPile.

## Features

- **Syntax highlighting** — TextMate grammar for instant coloring; LSP semantic tokens for richer, compiler-accurate highlighting when the server is running.
- **Diagnostics** — Real-time error reporting from the UniLogic lexer, parser, and semantic checker. Errors appear as red squiggles with line/column positions.
- **Hover definitions** — Hover over any UniLogic keyword to see a plain-English glossary definition.
- **Code folding** — Fold `function`/`end function`, `if`/`end if`, `while`/`end while`, `for`/`end for` blocks.
- **Comment toggling** — `Ctrl+/` toggles `//` line comments.
- **Bracket matching** — Auto-pairs `()`, `[]`, `""`.
- **Auto-indent** — Indents after `function`, `if`, `else`, `while`, `for`; dedents on `end` and `else`.

## Requirements

- **Python 3.10+** with `pygls` installed:
  ```
  pip install pygls
  ```
- The XPile compiler source tree (this extension expects to live at `XPile 3-17/LSP/vscode-ul/`).

## Install

```bash
# 1. Install the Node dependency
cd "XPile 3-17/LSP/vscode-ul"
npm install

# 2. Package the extension
npx @vscode/vsce package --allow-missing-repository

# 3. Install the .vsix into VS Code
code --install-extension unilogic-0.1.0.vsix
```

Or for development, symlink into your extensions folder:
```bash
# Windows (run as admin or with developer mode enabled)
mklink /J "%USERPROFILE%\.vscode\extensions\unilogic" "C:\Users\azt12\OneDrive\Documents\Computing\Pragma\CURRENT\XPile 3-17\LSP\vscode-ul"

# Then run: npm install   (inside the symlinked folder)
# Reload VS Code.
```

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| `unilogic.pythonPath` | `"python"` | Path to the Python interpreter for the LSP server |
| `unilogic.lsp.enabled` | `true` | Enable/disable the language server |

## File Structure

```
XPile 3-17/
  LSP/
    lsp.py                  <- Language server (Python, pygls)
    vscode-ul/              <- This extension
      package.json
      extension.js          <- Launches lsp.py over stdio
      language-configuration.json
      syntaxes/
        ul.tmLanguage.json  <- TextMate grammar fallback
      README.md
```
