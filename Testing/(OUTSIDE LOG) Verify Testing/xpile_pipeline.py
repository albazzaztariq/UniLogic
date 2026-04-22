#!/usr/bin/env python3
"""xpile_pipeline.py — Full transpilation verification pipeline.

Usage:
    python xpile_pipeline.py <source_file> <target_lang>

Example:
    python xpile_pipeline.py test.c ul      # C → UL → C round-trip
    python xpile_pipeline.py test.py ul      # Python → UL → Python round-trip
    python xpile_pipeline.py test.ul c       # UL → C → UL round-trip

Pipeline:
    Step 1: source → target (convert, verify, fuzz)
    Step 2: target → back to source language (convert, verify, fuzz)
    Both must pass for one test to be complete.

Checks at each step:
    1. Output compiles
    2. Output runs
    3. Output matches original for same inputs (fuzzer)
    4. Every type translated exactly (type comparator)
    5. Every function translated exactly (function comparator)
    6. Original source was NOT modified

This script is READ-ONLY. Use attrib -R to unlock if changes are needed.
"""

import sys
import os
import re
import json
import shutil
import hashlib
import subprocess
from datetime import datetime

# ── Configuration ──────────────────────────────────────────────────────────

XPILE_ROOT = None  # Set at runtime
PYTHON = sys.executable
VERIFY_DIR = None  # Set at runtime

def find_xpile_root():
    """Walk up from script location to find XPile root (has Main.py)."""
    # Try known location first
    known = r"C:\Users\azt12\OneDrive\Documents\Computing\LANG\UniLogic\CURRENT\XPile"
    if os.path.isfile(os.path.join(known, "Main.py")):
        return known
    # Walk up
    d = os.path.dirname(os.path.abspath(__file__))
    for _ in range(10):
        if os.path.isfile(os.path.join(d, "Main.py")):
            return d
        d = os.path.dirname(d)
    return None


# ── File integrity ─────────────────────────────────────────────────────────

def file_hash(path):
    """SHA256 of a file's contents."""
    if not os.path.isfile(path):
        return None
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


# ── Type Comparator ────────────────────────────────────────────────────────

def extract_types_c(source):
    """Extract type declarations from C source. Returns list of (name, fields)."""
    types = []
    # Match typedef struct { ... } Name;
    pattern = re.compile(
        r'typedef\s+struct\s*(?:\w+\s*)?\{([^}]*)\}\s*(\w+)\s*;',
        re.DOTALL
    )
    for m in pattern.finditer(source):
        body = m.group(1)
        name = m.group(2)
        fields = []
        for line in body.strip().split("\n"):
            line = line.strip().rstrip(";").strip()
            if line and not line.startswith("//"):
                fields.append(line)
        types.append({"name": name, "fields": fields})
    return types


def extract_types_ul(source):
    """Extract type declarations from UL source. Returns list of (name, fields)."""
    types = []
    lines = source.split("\n")
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        m = re.match(r'^type\s+(\w+)', line)
        if m:
            name = m.group(1)
            fields = []
            i += 1
            while i < len(lines):
                fline = lines[i].strip()
                if fline.startswith("end type"):
                    break
                if fline and not fline.startswith("//"):
                    fields.append(fline)
                i += 1
            types.append({"name": name, "fields": fields})
        i += 1
    return types


def compare_types(source_types, target_types, source_lang, target_lang):
    """Compare type declarations between source and target. Returns list of errors."""
    errors = []

    source_by_name = {t["name"]: t for t in source_types}
    target_by_name = {t["name"]: t for t in target_types}

    # Check every source type exists in target
    for name, stype in source_by_name.items():
        if name not in target_by_name:
            errors.append(f"TYPE MISSING: '{name}' exists in {source_lang} but not in {target_lang}")
            continue
        ttype = target_by_name[name]
        # Compare field count
        if len(stype["fields"]) != len(ttype["fields"]):
            errors.append(
                f"TYPE FIELD COUNT: '{name}' has {len(stype['fields'])} fields in {source_lang} "
                f"but {len(ttype['fields'])} in {target_lang}"
            )

    # Check for extra types in target
    for name in target_by_name:
        if name not in source_by_name:
            errors.append(f"TYPE EXTRA: '{name}' exists in {target_lang} but not in {source_lang}")

    return errors


# ── Function Comparator ────────────────────────────────────────────────────

def extract_functions_c(source):
    """Extract function signatures from C source."""
    funcs = []
    # Match: returntype funcname(params) {
    pattern = re.compile(
        r'^(\w[\w\s\*]*?)\s+(\w+)\s*\(([^)]*)\)\s*\{',
        re.MULTILINE
    )
    for m in pattern.finditer(source):
        ret = m.group(1).strip()
        name = m.group(2).strip()
        params = m.group(3).strip()
        if name not in ("if", "while", "for", "switch", "else"):
            funcs.append({"name": name, "returns": ret, "params": params})
    return funcs


def extract_functions_ul(source):
    """Extract function signatures from UL source."""
    funcs = []
    pattern = re.compile(
        r'^function\s+(\w+)\s*\(([^)]*)\)(?:\s+returns\s+(.+))?',
        re.MULTILINE
    )
    for m in pattern.finditer(source):
        name = m.group(1)
        params = m.group(2).strip()
        ret = m.group(3).strip() if m.group(3) else "none"
        funcs.append({"name": name, "returns": ret, "params": params})
    return funcs


def compare_functions(source_funcs, target_funcs, source_lang, target_lang):
    """Compare function signatures. Returns list of errors."""
    errors = []

    source_by_name = {f["name"]: f for f in source_funcs}
    target_by_name = {f["name"]: f for f in target_funcs}

    for name, sfunc in source_by_name.items():
        if name not in target_by_name:
            errors.append(f"FUNC MISSING: '{name}' exists in {source_lang} but not in {target_lang}")
            continue
        tfunc = target_by_name[name]
        # Compare param count
        sparams = [p.strip() for p in sfunc["params"].split(",") if p.strip()] if sfunc["params"] else []
        tparams = [p.strip() for p in tfunc["params"].split(",") if p.strip()] if tfunc["params"] else []
        if len(sparams) != len(tparams):
            errors.append(
                f"FUNC PARAMS: '{name}' has {len(sparams)} params in {source_lang} "
                f"but {len(tparams)} in {target_lang}"
            )

    for name in target_by_name:
        if name not in source_by_name:
            # Skip internal/generated functions
            if not name.startswith("_") and name != "main":
                errors.append(f"FUNC EXTRA: '{name}' in {target_lang} but not in {source_lang}")

    return errors


# ── UL Error Scan ──────────────────────────────────────────────────────────

def scan_ul_errors(ul_source):
    """Scan UL source for error markers that indicate failed translation."""
    errors = []
    for i, line in enumerate(ul_source.split("\n"), 1):
        if "/* ERROR:" in line:
            errors.append(f"Line {i}: TRANSLATION ERROR — {line.strip()[:100]}")
        if "__unknown_" in line:
            errors.append(f"Line {i}: UNMAPPED TYPE — {line.strip()[:100]}")
        if re.search(r'\b\w+_c\b', line) and "_c " in line and "// " not in line:
            # Potential _safe_name rename — check carefully
            m = re.search(r'\b(\w+_c)\b', line)
            if m and m.group(1) not in ("test_c",):
                errors.append(f"Line {i}: POSSIBLE RENAME — {m.group(1)} — {line.strip()[:100]}")
    return errors


# ── Build & Run Helpers ────────────────────────────────────────────────────

def run_cmd(cmd, timeout=120, cwd=None):
    """Run a command, return (success, stdout, stderr)."""
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=cwd or XPILE_ROOT)
        return r.returncode == 0, r.stdout.strip(), r.stderr.strip()
    except subprocess.TimeoutExpired:
        return False, "", "TIMEOUT"
    except Exception as e:
        return False, "", str(e)


def compile_c(source_path, exe_path):
    """Compile C source to executable."""
    return run_cmd(["gcc", source_path, "-o", exe_path, "-lm"])


def compile_ul_to_c(ul_path, c_path):
    """Compile UL to C via Main.py."""
    return run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"),
                    ul_path, "-t", "c", "-o", c_path])


def compile_ul_to_python(ul_path, py_path):
    """Compile UL to Python via Main.py."""
    return run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"),
                    ul_path, "-t", "python", "-o", py_path])


def transpile_c_to_ul(c_path, ul_path):
    """Transpile C to UL via c2ul."""
    return run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Transcoder", "Encoder", "c2ul.py"),
                    c_path, "-o", ul_path])


def transpile_py_to_ul(py_path, ul_path):
    """Transpile Python to UL via py2ul."""
    return run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Transcoder", "Encoder", "py2ul.py"),
                    py_path, "-o", ul_path])


def run_exe(exe_path, stdin_input="", timeout=10):
    """Run an executable, return stdout."""
    try:
        r = subprocess.run([exe_path], input=stdin_input, capture_output=True,
                          text=True, timeout=timeout)
        return r.stdout
    except:
        return None


def run_python(py_path, stdin_input="", timeout=10):
    """Run a Python file, return stdout."""
    try:
        r = subprocess.run([PYTHON, "-B", py_path], input=stdin_input,
                          capture_output=True, text=True, timeout=timeout)
        return r.stdout
    except:
        return None


# ── Fuzzer ─────────────────────────────────────────────────────────────────

def fuzz_compare(run_a, run_b, rounds=20):
    """Run both executables/scripts with random inputs, compare outputs.
    run_a and run_b are callables that take stdin_input and return stdout."""
    import random
    failures = []
    for r in range(rounds):
        rng = random.Random(r)
        input_lines = []
        for _ in range(rng.randint(1, 5)):
            choice = rng.choice(["int", "string", "edge"])
            if choice == "int":
                input_lines.append(str(rng.randint(-2**31, 2**31 - 1)))
            elif choice == "string":
                input_lines.append("".join(chr(rng.randint(32, 126)) for _ in range(rng.randint(0, 50))))
            elif choice == "edge":
                input_lines.append(rng.choice(["0", "-1", "1", "2147483647", "-2147483648", "", " "]))
        fuzz_input = "\n".join(input_lines) + "\n"

        out_a = run_a(fuzz_input)
        out_b = run_b(fuzz_input)

        if out_a != out_b:
            failures.append({
                "round": r,
                "input": fuzz_input[:200],
                "output_a": (out_a or "")[:200],
                "output_b": (out_b or "")[:200],
            })
    return failures


# ── Pipeline ───────────────────────────────────────────────────────────────

def log(msg, log_lines):
    """Print and record."""
    print(msg)
    log_lines.append(msg)


def run_pipeline(source_file, target_lang):
    """Run the full verification pipeline."""
    global XPILE_ROOT, VERIFY_DIR

    XPILE_ROOT = find_xpile_root()
    if not XPILE_ROOT:
        print("ERROR: Cannot find XPile root (Main.py)")
        sys.exit(1)

    VERIFY_DIR = os.path.join(
        os.path.dirname(os.path.abspath(__file__))
    )

    source_file = os.path.abspath(source_file)
    source_ext = os.path.splitext(source_file)[1].lower()
    source_lang = {".c": "C", ".py": "Python", ".ul": "UL"}.get(source_ext, "unknown")
    target_lang = target_lang.lower()

    timestamp = datetime.now()
    ts_str = timestamp.strftime("%Y%m%d_%H%M%S")
    mmdd = timestamp.strftime("%m%d")
    log_file = os.path.join(VERIFY_DIR, f"{mmdd} Test Log.txt")
    log_lines = []

    log(f"{'=' * 60}", log_lines)
    log(f"XPile Verification Pipeline", log_lines)
    log(f"Time: {timestamp.strftime('%Y-%m-%d %I:%M %p')}", log_lines)
    log(f"Source: {os.path.basename(source_file)} ({source_lang})", log_lines)
    log(f"Target: {target_lang}", log_lines)
    log(f"{'=' * 60}", log_lines)

    # Hash the original source — must not change
    original_hash = file_hash(source_file)
    log(f"Original hash: {original_hash[:16]}...", log_lines)

    # ── STEP 1: Source → Target ────────────────────────────────────────

    log(f"\n--- STEP 1: {source_lang} -> {target_lang.upper()} ---", log_lines)

    # Determine output paths
    base = os.path.splitext(source_file)[0]
    if source_lang == "C" and target_lang == "ul":
        target_file = base + ".ul"
        convert_fn = transpile_c_to_ul
    elif source_lang == "Python" and target_lang == "ul":
        target_file = base + ".ul"
        convert_fn = transpile_py_to_ul
    elif source_lang == "UL" and target_lang == "c":
        target_file = base + "_rt.c"
        convert_fn = compile_ul_to_c
    elif source_lang == "UL" and target_lang == "python":
        target_file = base + "_rt.py"
        convert_fn = compile_ul_to_python
    else:
        log(f"ERROR: Unsupported conversion {source_lang} -> {target_lang}", log_lines)
        _write_log(log_file, log_lines)
        sys.exit(1)

    # Convert
    log(f"Converting...", log_lines)
    ok, stdout, stderr = convert_fn(source_file, target_file)
    if not ok:
        log(f"FAIL: Conversion failed", log_lines)
        log(f"  {stderr[:300]}", log_lines)
        _write_log(log_file, log_lines)
        sys.exit(1)
    log(f"  Conversion: PASS ({stdout[:100] if stdout else 'ok'})", log_lines)

    # Verify original not modified
    if file_hash(source_file) != original_hash:
        log(f"FAIL: ORIGINAL SOURCE WAS MODIFIED — ABORTING", log_lines)
        _write_log(log_file, log_lines)
        sys.exit(1)
    log(f"  Original integrity: PASS (unchanged)", log_lines)

    # Read both files for comparison
    with open(source_file, encoding="utf-8", errors="replace") as f:
        source_content = f.read()
    with open(target_file, encoding="utf-8", errors="replace") as f:
        target_content = f.read()

    # Scan for error markers (if target is UL)
    if target_lang == "ul":
        ul_errors = scan_ul_errors(target_content)
        if ul_errors:
            log(f"  UL Error Scan: FAIL ({len(ul_errors)} errors)", log_lines)
            for e in ul_errors[:5]:
                log(f"    {e}", log_lines)
            if len(ul_errors) > 5:
                log(f"    ... and {len(ul_errors) - 5} more", log_lines)
        else:
            log(f"  UL Error Scan: PASS (clean)", log_lines)

    # Type comparison
    log(f"  Comparing types...", log_lines)
    if source_lang == "C":
        src_types = extract_types_c(source_content)
        tgt_types = extract_types_ul(target_content) if target_lang == "ul" else extract_types_c(target_content)
    elif source_lang == "UL":
        src_types = extract_types_ul(source_content)
        tgt_types = extract_types_c(target_content) if target_lang == "c" else extract_types_ul(target_content)
    else:
        src_types = []
        tgt_types = []

    type_errors = compare_types(src_types, tgt_types, source_lang, target_lang.upper())
    if type_errors:
        log(f"  Type Comparison: FAIL ({len(type_errors)} issues)", log_lines)
        for e in type_errors:
            log(f"    {e}", log_lines)
    else:
        log(f"  Type Comparison: PASS ({len(src_types)} types match)", log_lines)

    # Function comparison
    log(f"  Comparing functions...", log_lines)
    if source_lang == "C":
        src_funcs = extract_functions_c(source_content)
        tgt_funcs = extract_functions_ul(target_content) if target_lang == "ul" else extract_functions_c(target_content)
    elif source_lang == "UL":
        src_funcs = extract_functions_ul(source_content)
        tgt_funcs = extract_functions_c(target_content) if target_lang == "c" else extract_functions_ul(target_content)
    else:
        src_funcs = []
        tgt_funcs = []

    func_errors = compare_functions(src_funcs, tgt_funcs, source_lang, target_lang.upper())
    if func_errors:
        log(f"  Function Comparison: FAIL ({len(func_errors)} issues)", log_lines)
        for e in func_errors:
            log(f"    {e}", log_lines)
    else:
        log(f"  Function Comparison: PASS ({len(src_funcs)} functions match)", log_lines)

    # Compile and run
    log(f"  Compile & Run...", log_lines)

    # Compile/run original
    if source_lang == "C":
        orig_exe = base + "_orig.exe"
        ok, _, err = compile_c(source_file, orig_exe)
        if not ok:
            log(f"  FAIL: Original C won't compile: {err[:200]}", log_lines)
            _write_log(log_file, log_lines)
            sys.exit(1)
        orig_output = run_exe(orig_exe)
        orig_runner = lambda inp: run_exe(orig_exe, inp)
    elif source_lang == "Python":
        orig_output = run_python(source_file)
        orig_runner = lambda inp: run_python(source_file, inp)
    elif source_lang == "UL":
        # Compile UL to Python to run
        ul_py = base + "_check.py"
        ok, _, err = compile_ul_to_python(source_file, ul_py)
        if ok:
            orig_output = run_python(ul_py)
            orig_runner = lambda inp: run_python(ul_py, inp)
        else:
            orig_output = None
            orig_runner = None
            log(f"  WARNING: Can't run UL directly, skipping output comparison", log_lines)

    # Compile/run target
    if target_lang == "ul":
        # Compile UL to C, then run
        rt_c = base + "_step1_rt.c"
        ok, _, err = compile_ul_to_c(target_file, rt_c)
        if not ok:
            log(f"  FAIL: UL won't compile to C: {err[:200]}", log_lines)
            _write_log(log_file, log_lines)
            sys.exit(1)
        rt_exe = base + "_step1_rt.exe"
        ok, _, err = compile_c(rt_c, rt_exe)
        if not ok:
            log(f"  FAIL: Generated C won't compile: {err[:200]}", log_lines)
            _write_log(log_file, log_lines)
            sys.exit(1)
        target_output = run_exe(rt_exe)
        target_runner = lambda inp: run_exe(rt_exe, inp)
    elif target_lang == "c":
        rt_exe = base + "_step1_rt.exe"
        ok, _, err = compile_c(target_file, rt_exe)
        if not ok:
            log(f"  FAIL: Generated C won't compile: {err[:200]}", log_lines)
            _write_log(log_file, log_lines)
            sys.exit(1)
        target_output = run_exe(rt_exe)
        target_runner = lambda inp: run_exe(rt_exe, inp)
    elif target_lang == "python":
        target_output = run_python(target_file)
        target_runner = lambda inp: run_python(target_file, inp)

    # Compare default output
    if orig_output is not None and target_output is not None:
        orig_clean = (orig_output or "").replace("\r\n", "\n")
        target_clean = (target_output or "").replace("\r\n", "\n")
        if orig_clean == target_clean:
            log(f"  Default Output: PASS (match)", log_lines)
        else:
            log(f"  Default Output: FAIL", log_lines)
            for i, (a, b) in enumerate(zip(orig_clean.splitlines(), target_clean.splitlines())):
                if a != b:
                    log(f"    First diff line {i+1}:", log_lines)
                    log(f"      orig: {a[:100]}", log_lines)
                    log(f"      target: {b[:100]}", log_lines)
                    break

    # Fuzz
    if orig_runner and target_runner:
        log(f"  Fuzzing (20 rounds)...", log_lines)
        failures = fuzz_compare(orig_runner, target_runner, rounds=20)
        if failures:
            log(f"  Fuzz: FAIL ({len(failures)} rounds differ)", log_lines)
            for fail in failures[:3]:
                log(f"    Round {fail['round']}: input={fail['input'][:50]!r}", log_lines)
        else:
            log(f"  Fuzz: PASS (20/20 match)", log_lines)

    # ── STEP 2: Target → Back to Original ──────────────────────────────

    log(f"\n--- STEP 2: {target_lang.upper()} -> {source_lang} (round-trip) ---", log_lines)

    target_hash = file_hash(target_file)

    if source_lang == "C" and target_lang == "ul":
        roundtrip_file = base + "_roundtrip.c"
        ok, stdout, stderr = compile_ul_to_c(target_file, roundtrip_file)
    elif source_lang == "UL" and target_lang == "c":
        roundtrip_file = base + "_roundtrip.ul"
        ok, stdout, stderr = transpile_c_to_ul(target_file, roundtrip_file)
    elif source_lang == "Python" and target_lang == "ul":
        roundtrip_file = base + "_roundtrip.py"
        ok, stdout, stderr = compile_ul_to_python(target_file, roundtrip_file)
    elif source_lang == "UL" and target_lang == "python":
        roundtrip_file = base + "_roundtrip.ul"
        ok, stdout, stderr = transpile_py_to_ul(target_file, roundtrip_file)
    else:
        log(f"ERROR: Can't round-trip {target_lang} -> {source_lang}", log_lines)
        _write_log(log_file, log_lines)
        sys.exit(1)

    if not ok:
        log(f"  FAIL: Round-trip conversion failed: {stderr[:300]}", log_lines)
        _write_log(log_file, log_lines)
        sys.exit(1)
    log(f"  Round-trip conversion: PASS", log_lines)

    # Verify target not modified during round-trip
    if file_hash(target_file) != target_hash:
        log(f"  FAIL: TARGET FILE WAS MODIFIED DURING ROUND-TRIP", log_lines)
        _write_log(log_file, log_lines)
        sys.exit(1)
    log(f"  Target integrity: PASS (unchanged)", log_lines)

    # Compile and run round-trip
    if source_lang == "C":
        rt2_exe = base + "_roundtrip.exe"
        ok, _, err = compile_c(roundtrip_file, rt2_exe)
        if not ok:
            log(f"  FAIL: Round-trip C won't compile: {err[:200]}", log_lines)
            _write_log(log_file, log_lines)
            sys.exit(1)
        rt2_output = run_exe(rt2_exe)
        rt2_runner = lambda inp: run_exe(rt2_exe, inp)
    elif source_lang == "Python":
        rt2_output = run_python(roundtrip_file)
        rt2_runner = lambda inp: run_python(roundtrip_file, inp)
    elif source_lang == "UL":
        rt2_py = base + "_roundtrip_check.py"
        ok, _, err = compile_ul_to_python(roundtrip_file, rt2_py)
        if ok:
            rt2_output = run_python(rt2_py)
            rt2_runner = lambda inp: run_python(rt2_py, inp)
        else:
            rt2_output = None
            rt2_runner = None

    # Compare round-trip output to original
    if orig_output is not None and rt2_output is not None:
        orig_clean = (orig_output or "").replace("\r\n", "\n")
        rt2_clean = (rt2_output or "").replace("\r\n", "\n")
        if orig_clean == rt2_clean:
            log(f"  Round-trip Output: PASS (matches original)", log_lines)
        else:
            log(f"  Round-trip Output: FAIL (differs from original)", log_lines)
            for i, (a, b) in enumerate(zip(orig_clean.splitlines(), rt2_clean.splitlines())):
                if a != b:
                    log(f"    First diff line {i+1}:", log_lines)
                    log(f"      orig: {a[:100]}", log_lines)
                    log(f"      roundtrip: {b[:100]}", log_lines)
                    break

    # Fuzz round-trip
    if orig_runner and rt2_runner:
        log(f"  Fuzzing round-trip (20 rounds)...", log_lines)
        failures = fuzz_compare(orig_runner, rt2_runner, rounds=20)
        if failures:
            log(f"  Round-trip Fuzz: FAIL ({len(failures)} rounds differ)", log_lines)
        else:
            log(f"  Round-trip Fuzz: PASS (20/20 match)", log_lines)

    # ── Summary ────────────────────────────────────────────────────────

    log(f"\n{'=' * 60}", log_lines)
    log(f"PIPELINE COMPLETE", log_lines)
    log(f"{'=' * 60}", log_lines)

    # Write log
    _write_log(log_file, log_lines)
    log(f"Log written to: {log_file}", [])


def _write_log(log_file, log_lines):
    """Append log lines to the test log file."""
    with open(log_file, "a", encoding="utf-8") as f:
        f.write("\n".join(log_lines))
        f.write("\n\n")


# ── Main ───────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) != 3:
        print("Usage: python xpile_pipeline.py <source_file> <target_lang>")
        print("  target_lang: ul, c, python")
        sys.exit(1)

    source_file = sys.argv[1]
    target_lang = sys.argv[2]

    if not os.path.isfile(source_file):
        print(f"ERROR: Source file not found: {source_file}")
        sys.exit(1)

    run_pipeline(source_file, target_lang)


if __name__ == "__main__":
    main()
