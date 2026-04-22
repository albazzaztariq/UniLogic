# XPile Transpilation Testing Requirements

## MANDATORY — No Exceptions

A transpilation test is NOT COMPLETE unless ALL of the following are verified:

### 1. Every type must instantiate identically
Every type in the source (int, float, double, char, uint32, structs, arrays, pointers) must:
- Exist in the transpiled output
- Be the same size/precision
- Behave identically under all operations

### 2. Every method/function must behave identically
Every function call in the source must:
- Exist in the transpiled output (no silent drops, no renames to nonexistent functions)
- Accept the same inputs
- Produce the same outputs
- Have the same side effects

### 3. Every variable and data structure must round-trip faithfully
- A variable declared as X in the source must be X in the output
- A struct with fields A, B, C must have the same fields in the same order
- An array of size N must be size N
- A pointer must remain a pointer

### 4. The four conditions (ALL required, not some):
1. **Compiles** — the transpiled output compiles without errors
2. **Runs** — the compiled binary executes without crashes
3. **Same output given same inputs** — byte-for-byte identical output
4. **Deterministic under varied inputs** — if you change inputs, the outputs change the same way in both versions

### 5. No silent substitutions
- `rand()` in C must produce the same sequence as `rand()` in the round-trip, not a different PRNG
- `printf("%x", n)` must produce hex output, not decimal
- `sizeof(type)` must return the same value
- Math functions must return the same floating-point result (within ULP)
- Sort order of equal elements must be preserved

### 6. No silent renames
- A C function named `left()` must remain callable as `left()` after round-trip
- If a name collision with a UL keyword exists, it must be flagged as an error, not silently renamed to a nonexistent function

---

## Testing Wrapper Tool

Every test must be run through an automated wrapper that:

1. **Compiles the original** source (C or Python) and captures output
2. **Transpiles** source → UL via c2ul or py2ul
3. **Checks for warnings** — any warning is logged, not ignored
4. **Compiles** UL → target language via Main.py
5. **Compiles** the generated target (gcc for C, python for Python)
6. **Runs** both original and round-trip with identical inputs
7. **Compares** output byte-for-byte
8. **Fuzzes inputs** — if the program accepts input, vary it and verify both versions produce identical output for each variation
9. **Type verification** — for every type declaration in the UL intermediate, verify the corresponding type exists and matches in the output
10. **Function verification** — for every function in the UL intermediate, verify it exists in the output and is callable
11. **Reports** every discrepancy, no summarizing, no "mostly passes"

### Input fuzzing
For programs that accept inputs (command-line args, stdin, function parameters):
- Generate N random valid inputs
- Run both original and round-trip with each input
- Compare outputs
- Report any divergence with the exact input that caused it

### Static verification
For the UL intermediate file:
- Scan for any `/* ERROR:` comments (indicates c2ul couldn't translate something)
- Scan for any `__unknown_` type prefixes (indicates unmapped types)
- Scan for any `_c` suffixed function names (indicates silent renames)
- Scan for any `0` literals that replaced function calls (legacy silent fallbacks)
- Verify every function declaration has a body or is a valid forward reference
- Verify every type used in declarations exists in the type table

---

## Stdlib Equivalence Map

Every C stdlib function that is mapped to a UL equivalent must be documented here with:
- Whether the mapping preserves exact behavior
- Whether the mapping changes output for any input
- Whether the C-target codegen emits the original C function (for round-trip fidelity)

Functions where UL's implementation DIFFERS from C's:
- `rand()` / `srand()` — different PRNG algorithm, different sequences
- `qsort()` — sort stability may differ
- Math functions — floating point precision may differ by ULP
- `printf` format specifiers — `%x`, `%c`, `%p`, `%u` not all preserved

For C-origin code round-tripping through UL back to C: the codegen MUST emit the original C stdlib function, not a UL substitute. The UL substitute is for code WRITTEN in UL that targets C.

---

## What "PASS" Means

A test PASSES only when:
- Zero warnings from transpiler
- Zero errors from compiler
- Zero crashes at runtime
- Output matches byte-for-byte
- No `/* ERROR: */` comments in intermediate files
- No `__unknown_` types in intermediate files
- No `_c` renamed functions in intermediate files

Anything else is a FAIL with a documented reason.

---

## Agent Session Monitor Tool (xpile_monitor)

This tool MUST be started the moment an agent BEGINS testing. It runs continuously throughout the session.

### On Start (arguments: xpile script path, origin file, target language):
1. Copy the XPile script being used (c2ul.py, codegen_c.py, etc.) to a session folder — this is the **pre-edit source of truth**
2. Copy the original origin file (the .c or .py being transpiled) to the same folder
3. Copy the generated target file (the .ul, _rt.c, _rt.py emitted by the compiler) to the same folder
4. Begin logging all file changes:
   - Line number
   - `Old Version:` followed by the old content
   - Newline
   - `New Version:` followed by the new content

### During Session:
- Log every edit to any monitored file in real-time
- Display changes in a visible terminal window, desktop app, or dashboard — THE USER MUST SEE THIS HAPPENING LIVE

### On Stop (stop command from agent):
1. Copy the XPile script again (now possibly modified) — append `_final` to filename
2. Copy the origin file again (possibly modified) — append `_final`
3. Copy the target/emitted file again (possibly modified) — append `_final`
4. All copies go to the same session folder
5. Generate a diff report between pre-edit and final versions of each file

### Session Folder Structure:
```
session_YYYYMMDD_HHMMSS/
  c2ul.py                    # pre-edit copy
  c2ul_final.py              # post-edit copy
  test_source.c              # original input (pre)
  test_source_final.c        # original input (post, if changed)
  test_output.ul             # generated UL (pre)
  test_output_final.ul       # generated UL (post)
  changes.log                # all edits with old/new content
  diff_report.txt            # summary of all diffs
```

---

## Transpilation Verification Tool (xpile_verify)

Standalone tool. Runnable from VS Code terminal or plain command line. The user gets the command, reviews, and approves each run.

### Command format:
```
python xpile_verify.py --source test.c --target c --ul test.ul [--fuzz N]
```

### What it does:
1. Compile original source, capture output
2. Transpile source → UL
3. Scan UL for error markers (`/* ERROR: */`, `__unknown_`, `_c` suffixes)
4. Compile UL → target
5. Compile generated target
6. Run both with identical inputs, diff outputs byte-for-byte
7. If `--fuzz N`: generate N random valid inputs, run both versions with each, compare
8. Type verification: for every type in UL, verify it exists in output
9. Function verification: for every function in UL, verify it exists in output
10. Report PASS or FAIL with exact details of every discrepancy

### Checklist (automated, every run):
- [ ] Source compiles
- [ ] c2ul/py2ul produces 0 warnings
- [ ] No `/* ERROR: */` in .ul
- [ ] No `__unknown_` types in .ul
- [ ] No `_c` suffixed function names in .ul
- [ ] UL compiles to target
- [ ] Target compiles (gcc/python)
- [ ] Output matches byte-for-byte
- [ ] Fuzz inputs produce matching outputs (if applicable)
- [ ] Every type in .ul has corresponding type in target
- [ ] Every function in .ul has corresponding function in target

---

## Reference: Eq@DFuzz

The differential fuzzing approach from [Eq@DFuzz](https://arxiv.org/html/2602.15761v1) is the closest existing tool to what we need. It generates inputs, runs both original and transformed code, and compares outputs to detect behavioral divergence. Our xpile_verify tool should implement this same approach for transpiled code equivalence checking.
