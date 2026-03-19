# Changelog

## v0.1.0 (2026-03-XX) — Initial Release

### Language
- 5 compilation targets: C, Python, JavaScript, LLVM IR, WebAssembly
- Native binary compilation via LLVM toolchain
- Bytecode VM (C++) for portable execution
- Type system: primitives, user types (structs), objects with inheritance
- Memory model: gc, manual, refcount, arena — per file via @dr
- Error handling: Result types, ? propagation, exit()
- Control flow: if/else, while, for each, match, yield/generators
- Array comprehensions
- Multiple return values, default parameters, variadic functions
- Pipe operator |>
- Inline assembly @asm blocks
- String and array method syntax (s.len(), arr.sort())
- Threading: spawn/wait/lock/unlock (@dr concurrency = threaded)
- Constants, @deprecated, @norm inline toggle

### Standard Library
- array, string, math — fully implemented
- map, file, http, json — FFI with C implementations

### Package Manager
- ul new, ul init, ul install, ul remove, ul list
- ul run, ul build, ul test, ul publish
- Registry at GitHub, SHA256 verification

### Tooling
- VS Code extension: syntax highlighting, diagnostics, hover,
  go-to-definition, autocomplete, profiler panels, DR monitor,
  normalization view, AI-powered explainers
- Project Manager (Qt C++): editor, REPL, DR configurator,
  build profiles, package manager tab, themes
- Installer: Windows x64, auto-detects Python, PATH setup
- ul command wrapper for Windows and Unix

### Website
- Interactive AI/ML optimization learning guide (20 sections, 6 widgets)
- Dynamic runtime reference with animated memory model widgets
- Features page with can't/can comparison cards
- Getting started guide
- Browser REPL
- DR conflict checker
