# UniLogic

One language for everything. UL compiles to C, Python, JavaScript,
LLVM IR, and WebAssembly from a single source file. Runtime behavior
— memory model, safety level, integer width, concurrency — is
configurable per file via DR settings.

## Quick Start

1. Install Python 3.10+
2. Clone this repo
3. Run a program:
   ```
   ul run hello.ul
   ```

## What it does

- Compiles to 5 targets from one source file
- Four memory models: gc, manual, refcount, arena — per file
- Result types with automatic error propagation
- Static types with optional dynamic mode
- Built-in profiler: --profile flag, VS Code flame graph panel
- Package manager: ul install, ul new, ul build, ul test

## Tools

- **VS Code Extension** — syntax highlighting, diagnostics,
  go-to-definition, AI-powered explainers, profiler panels
- **Project Manager** — Qt C++ GUI for building and managing projects
- **REPL** — interactive interpreter, also runs in browser

## Documentation

- [Website](https://albazzaztariq.github.io/UniLogic/)
- [Getting Started](https://albazzaztariq.github.io/UniLogic/getting-started.html)
- [Language Reference](https://albazzaztariq.github.io/UniLogic/language-main.html)
- [DR Settings](https://albazzaztariq.github.io/UniLogic/dynamic-runtime.html)

## Build from source

See [BUILD.md](GUI_Qt/BUILD.md) for Project Manager build instructions.
The compiler requires Python 3.10+. No build step needed.

## License

MIT
