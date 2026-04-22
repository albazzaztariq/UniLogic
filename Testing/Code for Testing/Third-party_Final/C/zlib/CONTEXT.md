# zlib Testing Session Context

**Date:** 2026-03-21
**Session goal:** Stress-test c2ul transpiler against madler/zlib.
**Source:** https://github.com/madler/zlib (cloned to `zlib-src/`)

---

## What Was Done

1. Cloned madler/zlib into `tests/RealWorld/C/zlib/zlib-src/`
2. Analyzed adler32.c, crc32.c, zutil.c for testable constructs
3. Created 4 standalone C test files (avoiding known walls like pointer arithmetic)
4. Ran each through full pipeline: `c2ul.py` -> `Main.py -t c` -> `gcc` -> diff
5. Fixed 5 bugs (3 in c2ul.py, 2 in codegen_c.py)
6. Documented results in RESULTS.md

---

## Test Results Summary

| Test | Result | Lines |
|------|--------|-------|
| `test_adler32_arith.c` | PASS | 9/9 |
| `test_crc32.c` | PARTIAL | 16/19 (unsigned overflow) |
| `test_zutil.c` | PASS | 21/21 |
| `test_macro_expand.c` | PASS | 11/11 |

---

## Bugs Fixed

### c2ul.py

**Bug 1: `(0, 0)` regex too broad** (`_gcc_preprocess`, line ~1355)
- Old: `re.sub(r'\(0\s*,\s*0\)', '0', source)` — replaced ANY `(0, 0)` including function args
- Effect: `func(0, 0)` became `func0` (a bare identifier), causing UL compile error
- Fix: `re.sub(r'(?<=[|][|]|[&][&])\s*\(\s*0\s*,\s*0\s*\)', ' 0', source)` — only after `||`/`&&`

**Bug 2: Global const array init lost** (`_visit_global_decl`)
- Old: `InitList` emitted as `array int name = 0`
- Effect: `static const unsigned long crc32_table[16] = {...}` became `array int crc32_table = 0`
- Fix: Check if `InitList` children are non-`NamedInitializer` scalars → emit `[v1, v2, ...]` literal

**Bug 3: `case N: break;` treated as fall-through** (`_visit_switch`)
- Old: any case with empty `real_stmts` was added to `current_vals` (fall-through accumulator)
- Effect: `case 2: break; case 4: flag=1;` became `if sz==2 or sz==4: flag=1` (wrong)
- Fix: Distinguish `true_fallthrough` (stmts=[], no break) from explicit-break no-op (`has_explicit_break`)
  → explicit-break no-op cases emit as empty-body group; true fall-through still accumulates

### codegen_c.py

**Bug C: `array string` indexing uses `str_char_at`** (Index codegen, line ~1639)
- Old: `if vtype.name == 'string'` — fires for `array string` too (element type name is 'string')
- Effect: `errmsg[idx]` (where errmsg is `char**`) got emitted as `str_char_at(errmsg, idx)` — wrong type
- Fix: Added `and not vtype.is_array` guard → falls through to plain `name[index]` emission

**Bug D: Struct var init `= 0` invalid in C** (VarDecl codegen, line ~969)
- Old: `self.emit(f"{ctype} {node.name} = {val};")` — emits `Point result = 0;` (gcc rejects)
- Effect: All local struct variables with zero-init failed to compile
- Fix: When `node.type_.name in self.type_decls` and `val == "0"`, emit `= {0}` (C aggregate init)

---

## What Works After This Session

- Bit operations: `bit_and`, `bit_or`, `bit_xor`, `bit_left`, `bit_right` — all correct
- Modulo arithmetic: `%` with large bases
- While loops, do-while loops, for-as-while
- If/else-if chains (any depth)
- Switch/case: fall-through grouping, no-op break cases, default
- Global const int array literals (16+ elements)
- Global const string / constant string
- Array of strings (`array string name = [...]`), indexing
- Struct typedef: field declaration, local init, field assign, pass-by-value return
- Typedef int aliases (uLong, uInt)
- Macro-expanded compound statements (gcc -E handles macros, c2ul sees plain C)
- Multiple function calls in same expression
- Nested function calls as printf args

---

## What Still Doesn't Work (Walls)

- **Pointer arithmetic** (`*p++`, `p - base`): c2ul maps `unsigned char *buf` to `string`;
  `buf + 1` becomes `__ul_strcat(buf, 1)` (wrong). The adler32_z() byte-walk loop is blocked.
- **Function pointer callbacks**: `apply(int fn, ...)` + `fn(x,y)` — UL semcheck rejects calling
  an `int` as a function. No UL syntax for function-type variables.
- **Unsigned 32-bit overflow**: UL int is signed 32-bit. CRC32 table values > 2^31-1 wrap negative.
  3 CRC32 test lines fail due to this. No fix without UL gaining `uint32`/`uint64` types.
- **sizeof/offsetof**: Returns `0` placeholder.
- **C stdlib with no UL equiv**: `sprintf`, `memset`, `qsort` dropped as `0`/no-op.

---

## Files Modified (Bugs Fixed)

| File | Change |
|------|--------|
| `Tools/c2ul.py` | Bug 1: narrow `(0,0)` assert regex |
| `Tools/c2ul.py` | Bug 2: emit global array InitList as `[v1, v2, ...]` |
| `Tools/c2ul.py` | Bug 3: distinguish `case N: break;` from true fall-through |
| `Codegen/codegen_c.py` | Bug C: `array string` index uses direct `[i]` not `str_char_at` |
| `Codegen/codegen_c.py` | Bug D: struct var `= 0` emits `= {0}` in C |

---

## What Next Agent Picks Up

The main remaining walls for zlib:
1. **Unsigned integer types**: Without `uint32`/`uint64`, any C code using values > 2^31-1 will
   produce wrong results. Adding `uint32` to UL would unblock crc32.c fully.
2. **Pointer arithmetic**: The adler32_z() inner loop (`adler += *buf++; buf += 16;`) is the
   standard zlib pattern for walking byte arrays. UL needs a mutable-pointer or byte-slice concept.
3. **Function pointers**: zutil's `alloc_func`/`free_func` callback pattern (zlib's custom allocator)
   cannot be expressed. UL would need first-class function values or a lambda/closure type.
4. **Full file round-trip**: Attempt `adler32.c` or `zutil.c` full-file translation through
   `gcc -E | c2ul | Main.py -t c | gcc`. The prior session showed 775 gcc errors on cJSON.c
   mainly from type erasure; zlib files are smaller and may do better.

Next recommended test: `test_adler32_arith.c` with values extended to use `int64`/unsigned if
UL gains those types. Or attempt `adler32.c` full-file via `gcc -E`.
