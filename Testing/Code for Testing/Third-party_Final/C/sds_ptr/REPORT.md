# SDS Pointer Pattern Test Report

**Date:** 2026-03-21
**Session:** sds_ptr (new session, tests/RealWorld/C/sds_ptr/)
**Tool:** `Tools/c2ul.py` (C-to-UL) + `Main.py -t c` (UL-to-C)
**Pipeline:** C source -> c2ul -> .ul -> Main.py -t c -> .c -> gcc -> run -> compare vs original

---

## Summary

| Test | Pattern | c2ul | UL->C parse | gcc | Output | Result |
|------|---------|------|-------------|-----|--------|--------|
| test_sds_ptr_swap | Two-pointer backward walk (swap) | OK | OK | FAIL | -- | FAIL |
| test_sds_ptr_ll2str | Full sds_ll2str (pointer arithmetic + reversal) | OK | OK | FAIL | -- | FAIL |
| test_sds_ptr_dbl | Double-pointer output param | OK | FAIL | -- | -- | FAIL |
| test_sds_ptr_rebase | Pointer rebasing after realloc | OK | FAIL | -- | -- | FAIL |

**Tests attempted: 4 | PASS: 0 | FAIL: 4**

---

## Bugs Found

### Bug 1: char* byte-buffer pointers mapped to string type (c2ul.py)

**File:** `Tools/c2ul.py`, `map_type_ptr()` (PtrDecl branch, lines 97-109)

**Root cause:** c2ul maps all plain char* to string. This is correct for actual C strings but wrong
for byte-buffer pointers used as array cursors (e.g., char *p = s; *p++ = ...). When char*
becomes string, all downstream arithmetic and comparison operations break.

**Where it fires:**
- char *p, aux; -> string p = ""
- Function params char *s -> typed as string s
- Return type char * -> string

**Impact:** All 4 tests affected.

---

### Bug 2: string + integer emits __ul_strcat(s, 1) instead of s + 1 (codegen_c.py)

**File:** `Codegen/codegen_c.py`, line 1466-1468

**Root cause:** When the LHS of a + binary op is a string variable, codegen_c intercepts it and
emits __ul_strcat(). This is correct for actual string concatenation but wrong when the string
variable is a byte-buffer pointer (caused by Bug 1) and + 1 means pointer increment.

**Symptom:** s = s + 1 (from s++ pointer increment) -> s = __ul_strcat(s, 1) -- invalid C.

---

### Bug 3: string < string emits strcmp(s,p) < 0 instead of pointer comparison (codegen_c.py)

**File:** `Codegen/codegen_c.py`, line 1469-1477

**Root cause:** When both operands of < are string variables, codegen_c emits strcmp(). This is
correct for lexicographic string comparison but wrong for pointer comparison (caused by Bug 1).
while(s < p) (backward pointer walk) needs s < p as raw pointer comparison, not string ordering.

**Symptom:** while(s < p) -> while ((strcmp(s, p) < 0)) -- semantically wrong.

---

### Bug 4: _convert_printf truncates format string with embedded newline char (c2ul.py)

**File:** `Tools/c2ul.py`, `_visit_func_call_stmt()`, lines 1382-1406

**Root cause:** pycparser returns string constants with raw escape sequences. When the format
contains backslash-quote (escaped quote), the current accumulator ends up containing backslash-
quote + actual-newline-char. The emitted f'"{current}"' writes a literal newline into the .ul
file, breaking the print statement.

**Symptom:** printf("after grow: \"%s\"
", buf) -> UL line 59: print "after grow: \"" + buf +
(dangling + with no RHS).

**Impact:** UL parse error: test_sds_ptr_rebase.ul:61:6 -- unexpected token KW 'int'

**Trigger:** Any printf format string containing \" followed by more args and trailing 
.

---

### Bug 5: size(TypeKeyword) causes UL parse error (parser.py)

**File:** `Parser/parser.py`, parse_postfix / _CALLABLE_KW handling

**Root cause:** c2ul translates sizeof(char**) -> size(string). In UL, string is a keyword (KW
token). The size() call handler expects its argument as an expression, but keywords are not
accepted as plain identifier expressions by the parser.

**Symptom:** string <result> = cast(memtake((count * size(string))), string) -> parse error
test_sds_ptr_dbl.ul:40:54 -- unexpected token KW 'string'.

---

### Bug 6: char buf[N] arrays translate to array int instead of array uint8 (c2ul.py)

**File:** `Tools/c2ul.py`, `_visit_local_decl()` (array type path)

**Root cause:** char maps to "int" in _map_type_specifier(). So char buf[32] -> array int buf[32].
When this int[] array is passed to a function expecting char*, gcc errors with incompatible pointer
type (int* vs char*).

**Impact:** All 4 tests (anywhere a char[] buffer is declared and passed to a char* param).

---

## New Pointer Syntax Status

**What now works (new since previous sds session):**
- deref(<p>) = value -- write through pointer: CORRECT
- deref(<p>) in expression -- read through pointer: CORRECT  
- address(x) -- take address: CORRECT
- int <p> pointer variable declarations: CORRECT
- (*p) = ... in generated C: CORRECT
- p = (p - 1) -- pointer subtraction emits correct C: CORRECT
- l = ((int)(p - s)) -- pointer subtraction for length: CORRECT
- do { ... } while (v) -- do-while emission: CORRECT
- p = s (pointer copy): CORRECT

**What remains blocked:**
- char* byte-buffer pointers mapped to string (Bug 1 causes Bugs 2+3)
- char buf[N] -> int[] type mismatch (Bug 6)
- s + 1 (pointer advance) -> __ul_strcat(s, 1) (Bug 2 -- consequence of Bug 1)
- s < p (pointer comparison) -> strcmp(s,p) < 0 (Bug 3 -- consequence of Bug 1)
- printf with embedded " in format string -> truncated UL print (Bug 4)
- size(TypeKeyword) -> parse error (Bug 5)
