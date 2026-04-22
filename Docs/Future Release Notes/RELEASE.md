# Release Checklist

## Prerequisites

- Python 3.10+ must be installed (installer detects its absence and offers to open python.org/downloads)
- MinGW-w64 or MSVC is required for native binary compilation (not bundled — user installs separately)
- Inno Setup 6.7.1+ for building the installer (`choco install innosetup`)
- `ul.bat` handles Python PATH detection automatically — searches PATH, py launcher, and %LOCALAPPDATA%\Programs\Python\

## Pre-release verification

### Compiler
- [ ] All regression tests pass: `python Main.py test`
- [ ] Native binary compilation works on Windows (requires MinGW-w64 or MSVC)
- [ ] All 5 targets produce correct output for test suite
- [ ] Bytecode VM passes core tests
- [ ] `ul run hello.ul` works from clean install
- [ ] `python Main.py --version` outputs `UniLogic 0.1.0`

### VS Code Extension
- [ ] Extension installs without errors: `code --install-extension unilogic-0.1.0.vsix`
- [ ] Syntax highlighting works on .ul files
- [ ] Diagnostics appear on save
- [ ] Go-to-definition works
- [ ] Run File (Ctrl+F5) works with compiler path set
- [ ] No errors in Extension Host output channel

### Project Manager
- [ ] Launches cold (no QSettings) — welcome screen appears
- [ ] Quick Setup wizard finds Python automatically
- [ ] Can create a new project
- [ ] Build tab produces correct output
- [ ] REPL tab connects to interpreter
- [ ] Theme switching applies immediately
- [ ] Command palette (Ctrl+P) lists all commands

### Installer (tested with Inno Setup 6.7.1)
- [ ] Compiles with zero errors: `ISCC.exe unilogic_setup.iss`
- [ ] Python check detects Python in PATH, py launcher, and %LOCALAPPDATA%
- [ ] Python missing dialog offers to open python.org
- [ ] VS Code extension is bundled (vsix file included)
- [ ] VS Code task only appears when VS Code is installed
- [ ] ul.bat is installed and added to PATH
- [ ] Start Menu shortcuts created
- [ ] Post-install: `ul --version` works from fresh cmd.exe
- [ ] Post-install: Project Manager launches from Start Menu

### Website
- [ ] No broken links
- [ ] Getting started page tested end-to-end
- [ ] All code examples use current syntax
- [ ] Features page reflects current state

### Distribution
- [ ] Version numbers consistent across all 6 files (0.1.0)
- [ ] CHANGELOG.md updated
- [ ] RELEASE_NOTES.md written
- [ ] GitHub release has: installer exe, vsix, Project Manager zip

## Release steps

1. Run this checklist — all items must be checked
2. Update version in all 6 files if bumping
3. Tag the release: `git tag v0.1.0`
4. Build release artifacts: `powershell scripts/make_release.ps1`
5. Upload to GitHub releases with RELEASE_NOTES.md content
6. Update website download links
