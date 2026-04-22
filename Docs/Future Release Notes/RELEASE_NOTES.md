# UniLogic 0.1.0

First public release of the UniLogic programming language and toolchain.

## What's included

**Language**
- Compiles to C, Python, JavaScript, LLVM IR, and WebAssembly
- Native binary compilation via LLVM toolchain
- Bytecode VM for portable execution
- Static type system with optional dynamic mode
- Four memory models: gc, manual, refcount, arena — configurable per file
- Result types with automatic error propagation
- Objects with inheritance and virtual dispatch
- Threading, generators, pattern matching

**Tools**
- VS Code extension with syntax highlighting, diagnostics, profiler panels
- Project Manager (native Windows GUI, 809KB exe)
- Package manager: ul install, ul new, ul build, ul test
- Interactive browser REPL

**Installation**
Run `UniLogic_Setup_0.1.0.exe`. Python 3.10+ required.

## Quick start

After installation, open Command Prompt and run:

```
ul new myproject
cd myproject
ul run
```

## Documentation

https://albazzaztariq.github.io/UniLogic/

## Known limitations

- Bytecode VM: 83% test pass rate (result types not yet fully supported)
- Native binary compilation requires MinGW-w64 or MSVC (not bundled)
- Linux/macOS: use ul.sh wrapper, no GUI installer yet
