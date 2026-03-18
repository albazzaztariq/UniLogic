# UniLogic

**Syntax reference, safety documentation, and compiler source.**

## Live Site

| Page | URL |
|------|-----|
| Home Page | https://albazzaztariq.github.io/UniLogic/ |

## What is UniLogic?

UL is a statically-typed systems language that transpiles to C, Python, JavaScript, and LLVM IR. Designed to be readable to anyone who has written code in any modern language, while providing access to the full hardware stack when needed.

Standard UL reads like natural language. Base UL adds memory management, pointers, and bitwise operations in the same file — no mode switch, no separate language.

## Compiler

Requires Python 3 and clang or gcc.

## Safety

UL targets mathematically proven memory safety and functional correctness via formal verification with SMT solvers. See the [Safety page](https://albazzaztariq.github.io/UniLogic/safety.html) for the full methodology.

## Dynamic Runtime

UL exposes 9 runtime settings — memory model, safety level, type system, integer width, concurrency, and more — declared per source file at compile time. See the [Dynamic Runtime page](https://albazzaztariq.github.io/UniLogic/dynamic-runtime.html) for all settings.

## Links

- [Features & Functionality](https://albazzaztariq.github.io/UniLogic/features.html)
- [Learning & Resources](https://albazzaztariq.github.io/UniLogic/learning.html)
- [Tools & Utilities](https://albazzaztariq.github.io/UniLogic/tools.html)
- [Browser REPL](https://albazzaztariq.github.io/UniLogic/repl.html)
