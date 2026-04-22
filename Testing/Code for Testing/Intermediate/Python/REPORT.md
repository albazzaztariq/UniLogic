# Phase 3A Test Report: py2ul Simple Real Python Scripts

## Test Scripts

| ID | Name | Lines | Description |
|----|------|-------|-------------|
| T300 | fizzbuzz | 61 | FizzBuzz with counting extensions |
| T301 | calculator | 63 | Integer calculator with recursion (factorial, GCD) |
| T302 | stats | 82 | Array statistics: sum, min, max, range, median, bubble sort |
| T303 | text_proc | 82 | String processing: reverse, palindrome, char count, to_upper (ord/chr) |
| T304 | number_utils | 96 | Primes, fibonacci, digit operations |

## Pipeline Results

| Script | py2ul | UL->Python | Python Run | UL->C | C Build | C Run |
|--------|-------|------------|------------|-------|---------|-------|
| T300 fizzbuzz | PASS | PASS | MATCH | PASS | PASS | MATCH |
| T301 calculator | PASS | PASS | MATCH | PASS | PASS | MATCH |
| T302 stats | PASS | PASS | MATCH | PASS | PASS | MATCH |
| T303 text_proc | PASS | PASS | MATCH | PASS | PASS | MATCH |
| T304 number_utils | PASS | PASS | MATCH | PASS | PASS | MATCH |

## Transpiler Fixes Made

### 1. `if __name__ == "__main__"` guard (py2ul.py)
- **Problem**: py2ul emitted the Python `if __name__` guard verbatim, which is not valid UL syntax. UL auto-calls `main()`.
- **Fix**: Added detection in `visit_If()` to recognize the `__name__` guard pattern and skip it entirely.
- **Location**: `Tools/py2ul.py`, `visit_If()` method

### 2. Bare `list` type mapping (py2ul.py)
- **Problem**: Python `list` annotation (without subscript) mapped to `array string` instead of `array int`.
- **Fix**: Changed default mapping of bare `list`/`List` to `array int` since integer arrays are the more common case.
- **Location**: `Tools/py2ul.py`, `map_type()` method

### 3. String character indexing in C codegen (codegen_c.py)
- **Problem**: `s[i]` where `s` is a string produced a C `char`, but UL string operations expect `char*`. This caused type mismatch errors when the character was passed to string functions or `__ul_strcat`.
- **Fix**: Added string type detection in the `Index` handler — when the target variable is of type `string`, emit `str_char_at(s, i)` instead of `s[i]`. Also changed `str_char_at` to always be emitted in the preamble (it's small and commonly needed).
- **Location**: `Codegen/codegen_c.py`, `gen_expr()` Index handler and preamble

### 4. `ord()` and `chr()` mapping in py2ul (py2ul.py)
- **Problem**: Python `ord()` and `chr()` were not mapped to UL equivalents — the transpiler emitted them as-is, causing "undefined function" errors in the UL compiler.
- **Fix**: `ord(s[i])` maps to `char_code(s, i)`, `ord(s)` maps to `char_code(s, 0)`, `chr(n)` maps to `char_from_code(n)`.
- **Location**: `Tools/py2ul.py`, `expr()` Call handler

### 5. `char_from_code` builtin added (semcheck, all codegens)
- **Problem**: UL had `char_code(s, i)` (equivalent to `ord`) but no reverse operation (equivalent to `chr`).
- **Fix**: Added `char_from_code(n)` as a new UL builtin. Emits `chr(n)` in Python, `String.fromCharCode(n)` in JS, and a `__ul_char_from_code` helper in C that allocates a 2-byte string.
- **Location**: `Semantic/semcheck.py` BUILTINS, `Codegen/codegen_c.py`, `Codegen/codegen_python.py`, `Codegen/codegen_js.py`

## Regression

Full test suite: 3/4 passing (same as before — `test_bitfield.ul` was already failing, pre-existing).

## Files Modified

- `Tools/py2ul.py` — 3 fixes: __name__ guard, list type, ord/chr mapping
- `Codegen/codegen_c.py` — 2 fixes: string indexing via str_char_at, char_from_code builtin
- `Codegen/codegen_python.py` — 1 fix: char_from_code builtin
- `Codegen/codegen_js.py` — 1 fix: char_from_code builtin
- `Semantic/semcheck.py` — 1 fix: char_from_code added to BUILTINS

## Notes

- All scripts are real Python with type annotations and use standard builtins (ord, chr, len, str, etc.).
- No test scripts were modified to work around transpiler/compiler limitations — all issues were fixed in the toolchain.
- All scripts have a `main()` entry point returning `int`.
