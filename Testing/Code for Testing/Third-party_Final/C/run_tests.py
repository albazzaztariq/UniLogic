#!/usr/bin/env python3
"""
Real-World C Validation Test Runner
Tests C → UL → C round-trip using c2ul transpiler.

Strategy:
1. Attempt full-file transpilation
2. If it fails, document the failure with diagnosis
3. Run working outputs through UL compiler back to C
4. Compile and verify with gcc
"""

import sys
import os
import subprocess
import shutil
import time

_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))

PYTHON = sys.executable
XPILE  = _XPILE_ROOT
RDIR   = os.path.join(_XPILE_ROOT, "Tests", "RealWorld", "C")

sys.path.insert(0, XPILE)
sys.path.insert(0, os.path.join(XPILE, "Tools"))

results = []


def run(cmd, cwd=None, timeout=30):
    """Run a command, return (returncode, stdout, stderr)."""
    try:
        r = subprocess.run(
            cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout
        )
        return r.returncode, r.stdout, r.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "TIMEOUT"
    except Exception as e:
        return -2, "", str(e)


def gcc_compile(sources, output, extra_flags=None):
    cmd = ["gcc", "-o", output] + sources + ["-lm"]
    if extra_flags:
        cmd += extra_flags
    return run(cmd)


def c2ul(c_file, ul_file):
    return run([PYTHON, os.path.join(XPILE, "Tools", "c2ul.py"), c_file, "-o", ul_file])


def ul2c(ul_file, c_file):
    return run([PYTHON, os.path.join(XPILE, "Main.py"), ul_file, "-t", "c", "-o", c_file])


def test_file(label, c_src, test_harness=None, extra_src=None, expected_output=None):
    """
    Full pipeline test for a single C source file.
    label: human-readable name
    c_src: path to C source file to transpile
    test_harness: path to a .c file with main() to test the functions
    extra_src: additional C files needed
    expected_output: if given, compare against this string
    """
    print(f"\n{'='*60}")
    print(f"TEST: {label}")
    print(f"  Source: {c_src}")

    basename = os.path.splitext(os.path.basename(c_src))[0]
    ul_out   = os.path.join(RDIR, f"{basename}.ul")
    c_out    = os.path.join(RDIR, f"{basename}_out.c")
    orig_exe = os.path.join(RDIR, f"orig_{basename}.exe")
    trans_exe = os.path.join(RDIR, f"trans_{basename}.exe")

    result = {
        "label": label,
        "file": c_src,
        "c2ul": None,
        "ul2c": None,
        "orig_compile": None,
        "trans_compile": None,
        "orig_run": None,
        "trans_run": None,
        "match": None,
        "failures": [],
    }

    # Step 1: c2ul
    t0 = time.time()
    rc, out, err = c2ul(c_src, ul_out)
    elapsed = time.time() - t0
    if rc != 0:
        result["c2ul"] = "FAIL"
        result["failures"].append(f"c2ul failed: {err.strip()}")
        print(f"  [FAIL] c2ul ({elapsed:.2f}s): {err.strip()[:200]}")
        results.append(result)
        return result
    result["c2ul"] = f"OK ({elapsed:.2f}s)"
    print(f"  [OK]   c2ul ({elapsed:.2f}s): {out.strip()}")

    # Step 2: ul2c
    t0 = time.time()
    rc, out, err = ul2c(ul_out, c_out)
    elapsed = time.time() - t0
    if rc != 0:
        result["ul2c"] = "FAIL"
        result["failures"].append(f"ul2c failed: {(out+err).strip()}")
        print(f"  [FAIL] ul2c ({elapsed:.2f}s): {(out+err).strip()[:200]}")
        results.append(result)
        return result
    result["ul2c"] = f"OK ({elapsed:.2f}s)"
    print(f"  [OK]   ul2c ({elapsed:.2f}s)")

    # Step 3: Compile original (if test harness given)
    if test_harness:
        orig_sources = [c_src, test_harness]
        if extra_src:
            orig_sources += extra_src
        rc, out, err = gcc_compile(orig_sources, orig_exe)
        if rc != 0:
            result["orig_compile"] = "FAIL"
            result["failures"].append(f"original gcc compile failed: {err.strip()}")
            print(f"  [FAIL] orig compile: {err.strip()[:200]}")
            results.append(result)
            return result
        result["orig_compile"] = "OK"
        print(f"  [OK]   orig compile")

        # Step 4: Compile transpiled
        trans_sources = [c_out, test_harness]
        if extra_src:
            trans_sources += extra_src
        rc, out, err = gcc_compile(trans_sources, trans_exe)
        if rc != 0:
            result["trans_compile"] = "FAIL"
            result["failures"].append(f"transpiled gcc compile failed: {err.strip()}")
            print(f"  [FAIL] trans compile: {err.strip()[:300]}")
            results.append(result)
            return result
        result["trans_compile"] = "OK"
        print(f"  [OK]   trans compile")

        # Step 5: Run both and compare
        rc_o, orig_out, orig_err = run([orig_exe])
        rc_t, trans_out, trans_err = run([trans_exe])
        result["orig_run"] = f"exit={rc_o}"
        result["trans_run"] = f"exit={rc_t}"

        if orig_out == trans_out:
            result["match"] = "PASS"
            print(f"  [PASS] Output match! ({len(orig_out)} chars)")
        else:
            result["match"] = "FAIL"
            result["failures"].append(f"Output mismatch.\nORIG:\n{orig_out[:500]}\nTRANS:\n{trans_out[:500]}")
            print(f"  [FAIL] Output mismatch!")
            print(f"    ORIG:  {orig_out[:200]!r}")
            print(f"    TRANS: {trans_out[:200]!r}")
    else:
        # No harness — just check that transpiled output compiles with gcc
        rc, out, err = gcc_compile([c_out], trans_exe, extra_flags=["-c", "-fsyntax-only"])
        if rc != 0:
            # Try actual compile
            rc2, out2, err2 = run(["gcc", "-c", c_out, "-o", os.path.join(RDIR, f"{basename}_out.o")])
            if rc2 != 0:
                result["trans_compile"] = "FAIL"
                result["failures"].append(f"transpiled gcc -c failed: {err2.strip()}")
                print(f"  [FAIL] trans gcc -c: {err2.strip()[:300]}")
            else:
                result["trans_compile"] = "OK (object file)"
                print(f"  [OK]   trans gcc -c")
        else:
            result["trans_compile"] = "OK"
            print(f"  [OK]   trans syntax check")

    results.append(result)
    return result


# ============================================================
# TEST 1: cmark/cmark_ctype.c — simplest file, pure functions
# ============================================================

harness_ctype = os.path.join(RDIR, "test_cmark_ctype.c")
test_file(
    "cmark/cmark_ctype.c",
    os.path.join(RDIR, "cmark", "src", "cmark_ctype.c"),
    test_harness=harness_ctype,
)

# ============================================================
# TEST 2: cmark/buffer.c — struct-heavy, expected to fail at c2ul
# ============================================================

test_file(
    "cmark/buffer.c (struct-heavy, expected partial failure)",
    os.path.join(RDIR, "cmark", "src", "buffer.c"),
)

# ============================================================
# TEST 3: cJSON.c — large single-file library
# ============================================================

test_file(
    "cJSON/cJSON.c (large single-file, expected failures)",
    os.path.join(RDIR, "cJSON", "cJSON.c"),
)

# ============================================================
# TEST 4: SDS/sds.c — dynamic string library
# ============================================================

test_file(
    "sds/sds.c (dynamic strings, expected failures)",
    os.path.join(RDIR, "sds", "sds.c"),
)

# ============================================================
# SUMMARY
# ============================================================

print(f"\n{'='*60}")
print("SUMMARY")
print(f"{'='*60}")
for r in results:
    label = r["label"]
    status = r["match"] or r["trans_compile"] or r["ul2c"] or r["c2ul"] or "UNKNOWN"
    failures = len(r["failures"])
    print(f"  {label}: {status} ({failures} failures)")
    for f in r["failures"]:
        print(f"    - {f[:120]}")
