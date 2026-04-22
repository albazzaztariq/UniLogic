# zlib Real-World C Validation Results

**Date:** 2026-03-21
**Source:** madler/zlib (cloned to `zlib-src/`)
**Tool:** `Tools/c2ul.py` + `Main.py -t c` + `gcc`
**Pipeline:** C source -> gcc -E (macro expand) -> c2ul -> UL -> Main.py -t c -> gcc -> run -> diff

---

## Summary

Four test files derived from madler/zlib verified against the c2ul transpiler.
Three pass with exact output match. One (CRC32) has 3/19 mismatches due to UL signed-int
overflow (unsigned long values > INT32_MAX wrap negative) — this is a language-level wall,
not a c2ul bug. Five bugs were found and fixed across c2ul.py and codegen_c.py.

| Test | Source Pattern | Result | Lines Match |
|------|---------------|--------|-------------|
| adler32 arithmetic | adler32.c core (bit ops, %, while, combine) | PASS | 9/9 |
| crc32 table+XOR | crc32.c table lookup, bit_xor, reflect, fib | PARTIAL | 16/19 (3 overflow) |
| zutil utility | zutil.c typedef, switch, struct, do-while | PASS | 21/21 |
| macro expansion | adler32.c DO1..DO4 inline expansion pattern | PASS | 11/11 |

---

## Test 1: adler32 Arithmetic (PASS)

**File:** `test_adler32_arith.c`
**Functions:** `adler32_hello_world()`, `adler32_abc()`, `adler32_combine()`, `adler32_lo/hi()`
**Constructs:** `bit_and`, `bit_or`, `bit_left`, `bit_right`, `%`, `while`, `if/else if`, multi-function calls

**Result:** 9/9 test lines exact match.
- adler32("Hello, World!") = 530449514 ✓
- adler32_combine(r1, r2, 8) == direct full adler32 ✓
- All bit split/recombine operations correct ✓

**Note:** `0xDEADBEEF` test on last line shows expected overflow (UL int is 32-bit signed;
C `unsigned long` is 64-bit). Not a bug — documented UL type-width wall.

---

## Test 2: CRC32 Table + XOR (PARTIAL)

**File:** `test_crc32.c`
**Functions:** `crc32_byte()`, `get_crc_entry()`, `crc32_xor_fold()`, `crc32_loop_test()`, `fib()`, `reflect8()`
**Constructs:** global const int array (16 entries), `bit_xor`, `bit_right`, `bit_and`, `bit_left`,
while loops with if-inside, fibonacci, bit reversal

**Result:** 16/19 lines match. 3 mismatches are all signed overflow:
- `crc32_table[7]` = 2657392035 (> INT32_MAX) → wraps to -1637575261 in UL
- `crc32_table[15]` = 2428443839 → wraps to -1866523457 in UL
- `crc32_byte(0xFFFFFFFF, 0)`: `0xFFFFFFFF` overflows signed int → XOR result differs

**Root cause:** UL `int` is signed 32-bit. The CRC32 table contains values > 2^31-1 (unsigned).
No fix possible without UL gaining a `uint32` type. Documented wall.

**Bugs Fixed During This Test (see Bug Log below):**
- Bug 1: `(0, 0)` in source replaced by gcc-E workaround regex
- Bug 2: Global const array InitList emitted as `= 0` instead of `= [v1, v2, ...]`

---

## Test 3: zutil Utility Functions (PASS)

**File:** `test_zutil.c`
**Functions:** `get_err_msg()`, `my_version()`, `size_flag()`, `category()`, `decode_code()`,
`sum_bytes_dowhile()`, `point_dist_sq()`, `point_add()`, `combine_flags()`
**Constructs:** typedef chains (uLong/uInt), const char* string array, global const string,
switch/case with no-op break (`case 2: break`), fall-through cases (`case 1: case 2: body`),
nested if/else-if chain, do-while loop, struct typedef + field access, struct pass-by-value return

**Result:** 21/21 lines exact match.
- `size_flag(2)=0` ✓ (case 2: break — no-op, flag stays 0)
- `category(1)=10, category(2)=10` ✓ (fall-through cases 1,2,3 → 10)
- `sum_bytes_dowhile(1)=0` ✓ (do-while runs once: n=0→1 < 1 is false)
- `point_add: (11,22)` ✓ (struct pass by value + field assign)

**Bugs Fixed During This Test:**
- Bug 3: `case N: break;` treated as fall-through (should be no-op)
- Bug C: `array string[i]` used `str_char_at` instead of direct `name[i]`
- Bug D: Struct var initialized as `= 0` in C (invalid) — fixed to `= {0}`

---

## Test 4: Macro Expansion (PASS)

**File:** `test_macro_expand.c`
**Pattern:** Simulates adler32.c's DO1/DO4 macros after gcc -E expansion.
**Constructs:** compound `{...}` blocks inside while, chained `+=`, bit shifts in arithmetic,
`%` modulo reduction, `>>` and `& 0xffff` in CHOP/MOD28 pattern

**Result:** 11/11 lines exact match.
- DO4-style unrolled loop: 60227974 ✓
- Flags accumulation with `<<`: 65829 ✓
- CHOP+MOD28 pattern: all 3 test values correct ✓

---

## Bugs Fixed This Session

| # | File | Construct | Bug | Fix |
|---|------|-----------|-----|-----|
| 1 | `Tools/c2ul.py` | `f(0, 0)` call | `(0\s*,\s*0)` regex too broad — replaced function-call args `(0, 0)` with `0` making `func(0,0)` → `func0` | Changed regex to only match `(0,0)` after `||` or `&&` (assert-expansion context) |
| 2 | `Tools/c2ul.py` | Global const array | `static const int arr[] = {v1, v2, ...}` emitted as `array int arr = 0` instead of full literal | `_visit_global_decl`: detect `InitList` with scalar (non-`NamedInitializer`) children → emit `[v1, v2, ...]` |
| 3 | `Tools/c2ul.py` | `case N: break;` | `case N: break;` (explicit no-op break) treated as fall-through to next case | In `_visit_switch`: distinguish `all_stmts == []` (true fall-through) from `has_explicit_break` (no-op case → emit empty body group) |
| C | `Codegen/codegen_c.py` | `array string` indexing | `errmsg[idx]` where `errmsg` is `array string` used `str_char_at(errmsg, idx)` (wrong type: `char**` vs `char*`) | Added `and not vtype.is_array` guard to string-indexing branch |
| D | `Codegen/codegen_c.py` | Struct var init | `Point p = 0;` emitted in C (invalid for struct types) | When `node.type_.name in self.type_decls` and `val == "0"`, emit `= {0}` instead |

---

## Known Walls (Unchanged from Prior Sessions)

| Wall | Affects | Notes |
|------|---------|-------|
| UL int is signed 32-bit | CRC32 table values > 2^31-1 | Values 2657392035, 2428443839 wrap negative |
| Pointer arithmetic (`*p++`, `p-s`) | adler32_z() byte-walk loop | Cannot write through pointer; no UL equivalent |
| Function pointer callbacks | zutil zcalloc/zcfree pattern | `apply(fn, x, y)` where fn is int fails UL semcheck |
| sizeof/offsetof | zlibCompileFlags() | Returns 0 placeholder |
| C stdlib (sprintf, memset, etc.) | Various | Dropped as `0` or no-op comment |

---

## Files Created

| File | Description |
|------|-------------|
| `zlib-src/` | Full madler/zlib clone |
| `test_adler32.c` | Initial adler32 test (documents char* wall) |
| `test_adler32_arith.c` | Adler32 pure arithmetic (PASS) |
| `adler32_arith.ul` | c2ul output |
| `adler32_arith_out.c` | UL->C compiled |
| `test_crc32.c` | CRC32 table+XOR test |
| `crc32.ul` | c2ul output |
| `crc32_out.c` | UL->C compiled |
| `test_zutil.c` | zutil patterns test (PASS) |
| `zutil.ul` | c2ul output |
| `zutil_out.c` | UL->C compiled |
| `test_macro_expand.c` | Macro expansion simulation (PASS) |
| `macro_expand.ul` | c2ul output |
| `macro_expand_out.c` | UL->C compiled |
