# Pragma Language

**Syntax reference, safety documentation, and compiler source.**

## Live Site

| Page | URL |
|------|-----|
| Syntax Reference | https://pragma-project.github.io/Pragma-Language/syntax-reference.html |
| Safety & MemProof | https://pragma-project.github.io/Pragma-Language/safety.html |

## What is Pragma?

Pragma is a statically-typed systems language that transpiles to C. Designed to be readable to anyone who has written code in any modern language, while providing access to the full hardware stack when needed.

Standard Pragma reads like natural language. Base Pragma adds memory management, pointers, and bitwise operations in the same file — no mode switch, no separate language.

## Compiler

Source in . Requires Python 3 and clang or gcc.



## Safety

Pragma targets mathematically proven memory safety and functional correctness via Frama-C with the WP plugin. See the [Safety page](https://pragma-project.github.io/Pragma-Language/safety.html) for the full methodology.

## Org

Part of the [Pragma Project](https://github.com/Pragma-Project) — Language, Metal (hardware optimization), and OS.
