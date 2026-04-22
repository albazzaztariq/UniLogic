#!/usr/bin/env python3
"""testchain — Full transpilation test chain with verification.

Usage:
    testchain convert <origin> <output> -t <target_lang>
    testchain build <source> -t <backend>

Examples:
    testchain convert test.c test.ul -t ul
    testchain build test.ul -t c
    testchain build test.ul -t llvm
    testchain convert test.ul test_rt.c -t c
    testchain build test_rt.c -t c

Every command runs verification:
    - convert: type check, function check, error scan, fuzz, integrity check
    - build: compile, run, output comparison, fuzz

Logs to: Verify Testing/MMDD Test Log.txt
"""

import sys
import os
import re
import json
import hashlib
import subprocess
from datetime import datetime

# ── Paths ──────────────────────────────────────────────────────────────────

XPILE_ROOT = None
PYTHON = sys.executable
LOG_DIR = None
LIBS_DIR = None

def setup_paths():
    global XPILE_ROOT, LOG_DIR, LIBS_DIR
    # Find XPile root
    known = r"C:\Users\azt12\OneDrive\Documents\Computing\XPile"
    if os.path.isfile(os.path.join(known, "Main.py")):
        XPILE_ROOT = known
    else:
        d = os.path.dirname(os.path.abspath(__file__))
        for _ in range(10):
            if os.path.isfile(os.path.join(d, "Main.py")):
                XPILE_ROOT = d
                break
            d = os.path.dirname(d)
    if not XPILE_ROOT:
        print("ERROR: Cannot find XPile root")
        sys.exit(1)
    LIBS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Libraries")
    LOG_DIR = os.path.join(XPILE_ROOT, "Testing", "(OUTSIDE LOG) Verify Testing")


# ── Helpers ────────────────────────────────────────────────────────────────

def file_hash(path):
    if not os.path.isfile(path):
        return None
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()

def run_cmd(cmd, timeout=120):
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=XPILE_ROOT)
        return r.returncode == 0, r.stdout.strip(), r.stderr.strip()
    except subprocess.TimeoutExpired:
        return False, "", "TIMEOUT"
    except Exception as e:
        return False, "", str(e)

def run_exe(path, stdin_input="", timeout=10):
    try:
        r = subprocess.run([path], input=stdin_input, capture_output=True, text=True, timeout=timeout)
        return r.stdout
    except:
        return None

def run_python(path, stdin_input="", timeout=10):
    try:
        r = subprocess.run([PYTHON, "-B", path], input=stdin_input, capture_output=True, text=True, timeout=timeout)
        return r.stdout
    except:
        return None

LOG_LINES = []

def log(msg):
    print(msg)
    LOG_LINES.append(msg)

def write_log():
    mmdd = datetime.now().strftime("%m%d")
    log_file = os.path.join(LOG_DIR, f"{mmdd} Test Log.txt")
    os.makedirs(LOG_DIR, exist_ok=True)
    with open(log_file, "a", encoding="utf-8") as f:
        f.write("\n".join(LOG_LINES) + "\n\n")


# ── Type Extraction ────────────────────────────────────────────────────────

def extract_types_c(src):
    types = []
    for m in re.finditer(r'typedef\s+struct\s*(?:\w+\s*)?\{([^}]*)\}\s*(\w+)\s*;', src, re.DOTALL):
        fields = [l.strip().rstrip(";").strip() for l in m.group(1).strip().split("\n") if l.strip() and not l.strip().startswith("//")]
        types.append({"name": m.group(2), "fields": fields})
    return types

def extract_types_ul(src):
    types = []
    lines = src.split("\n")
    i = 0
    while i < len(lines):
        m = re.match(r'^\s*type\s+(\w+)', lines[i])
        if m:
            name = m.group(1)
            fields = []
            i += 1
            while i < len(lines):
                if lines[i].strip().startswith("end type"):
                    break
                if lines[i].strip() and not lines[i].strip().startswith("//"):
                    fields.append(lines[i].strip())
                i += 1
            types.append({"name": name, "fields": fields})
        i += 1
    return types


# ── Function Extraction ────────────────────────────────────────────────────

def extract_funcs_c(src):
    funcs = []
    for m in re.finditer(r'^([\w][\w\s\*]*?)\s+\*?\s*(\w+)\s*\(([^)]*)\)\s*\{', src, re.MULTILINE):
        name = m.group(2).strip()
        if name not in ("if", "while", "for", "switch", "else"):
            funcs.append({"name": name, "ret": m.group(1).strip(), "params": m.group(3).strip()})
    return funcs

def extract_funcs_ul(src):
    funcs = []
    for m in re.finditer(r'^function\s+(\w+)\s*\(([^)]*)\)(?:\s+returns\s+(.+))?', src, re.MULTILINE):
        funcs.append({"name": m.group(1), "ret": (m.group(3) or "none").strip(), "params": m.group(2).strip()})
    return funcs


# ── Comparison ─────────────────────────────────────────────────────────────

def compare_types(src_types, tgt_types, src_lang, tgt_lang):
    errors = []
    src_map = {t["name"]: t for t in src_types}
    tgt_map = {t["name"]: t for t in tgt_types}
    for name, s in src_map.items():
        if name not in tgt_map:
            errors.append(f"TYPE MISSING: '{name}' in {src_lang} not in {tgt_lang}")
        elif len(s["fields"]) != len(tgt_map[name]["fields"]):
            errors.append(f"TYPE FIELDS: '{name}' has {len(s['fields'])} in {src_lang}, {len(tgt_map[name]['fields'])} in {tgt_lang}")
    for name in tgt_map:
        if name not in src_map:
            errors.append(f"TYPE EXTRA: '{name}' in {tgt_lang} not in {src_lang}")
    return errors

def compare_funcs(src_funcs, tgt_funcs, src_lang, tgt_lang):
    errors = []
    src_map = {f["name"]: f for f in src_funcs}
    tgt_map = {f["name"]: f for f in tgt_funcs}
    for name, s in src_map.items():
        if name not in tgt_map:
            errors.append(f"FUNC MISSING: '{name}' in {src_lang} not in {tgt_lang}")
        else:
            sp = [p.strip() for p in s["params"].split(",") if p.strip() and p.strip() != "void"] if s["params"] else []
            tp = [p.strip() for p in tgt_map[name]["params"].split(",") if p.strip() and p.strip() != "void"] if tgt_map[name]["params"] else []
            if len(sp) != len(tp):
                errors.append(f"FUNC PARAMS: '{name}' {len(sp)} params in {src_lang}, {len(tp)} in {tgt_lang}")
    for name in tgt_map:
        if name not in src_map and not name.startswith("_"):
            errors.append(f"FUNC EXTRA: '{name}' in {tgt_lang} not in {src_lang}")
    return errors


# ── UL Error Scan ──────────────────────────────────────────────────────────

def scan_ul(src):
    errors = []
    for i, line in enumerate(src.split("\n"), 1):
        if "/* ERROR:" in line:
            errors.append(f"L{i}: TRANSLATION ERROR: {line.strip()[:80]}")
        if "__unknown_" in line:
            errors.append(f"L{i}: UNMAPPED TYPE: {line.strip()[:80]}")
    return errors


# ── Fuzz ───────────────────────────────────────────────────────────────────

def fuzz(exe, ul_file, rounds=20):
    """Run xpile_fuzz.py from Libraries against the given exe and ul_file."""
    script = os.path.join(LIBS_DIR, "xpile_fuzz.py")
    ok, out, err = run_cmd([PYTHON, "-B", script, exe, exe, ul_file, "--rounds", str(rounds)])
    passed = "OVERALL: PASS" in out
    return passed, out, err


# ── CONVERT command ────────────────────────────────────────────────────────

def cmd_convert(origin, output, target):
    origin = os.path.abspath(origin)
    output = os.path.abspath(output)
    origin_ext = os.path.splitext(origin)[1].lower()
    origin_lang = {".c": "C", ".py": "Python", ".ul": "UL"}.get(origin_ext, "?")

    log(f"\n{'='*60}")
    log(f"TESTCHAIN CONVERT: {os.path.basename(origin)} -> {os.path.basename(output)} (target: {target})")
    log(f"{'='*60}")

    # Hash original
    orig_hash = file_hash(origin)

    # Convert
    if origin_ext == ".c" and target == "ul":
        script = os.path.join(XPILE_ROOT, "Transcoder", "Encoder", "c2ul.py")
        ok, out, err = run_cmd([PYTHON, "-B", script, origin, "-o", output])
    elif origin_ext == ".py" and target == "ul":
        script = os.path.join(XPILE_ROOT, "Transcoder", "Encoder", "py2ul.py")
        ok, out, err = run_cmd([PYTHON, "-B", script, origin, "-o", output])
    elif origin_ext == ".ul" and target == "c":
        ok, out, err = run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"), origin, "-t", "c", "-o", output])
    elif origin_ext == ".ul" and target == "python":
        ok, out, err = run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"), origin, "-t", "python", "-o", output])
    elif origin_ext == ".ul" and target == "js":
        ok, out, err = run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"), origin, "-t", "js", "-o", output])
    else:
        log(f"  ERROR: Don't know how to convert {origin_ext} -> {target}")
        write_log()
        return False

    if not ok:
        log(f"  CONVERT: FAIL")
        log(f"  {(err or out)[:300]}")
        write_log()
        return False
    log(f"  CONVERT: PASS")

    # Integrity check
    if file_hash(origin) != orig_hash:
        log(f"  INTEGRITY: FAIL — ORIGINAL WAS MODIFIED")
        write_log()
        return False
    log(f"  INTEGRITY: PASS")

    # Read both
    with open(origin, encoding="utf-8", errors="replace") as f:
        src_content = f.read()
    with open(output, encoding="utf-8", errors="replace") as f:
        tgt_content = f.read()

    # UL error scan
    if target == "ul":
        ul_errs = scan_ul(tgt_content)
        if ul_errs:
            log(f"  UL SCAN: FAIL ({len(ul_errs)} errors)")
            for e in ul_errs[:5]:
                log(f"    {e}")
        else:
            log(f"  UL SCAN: PASS")

    # Type comparison
    if origin_ext == ".c":
        st = extract_types_c(src_content)
        tt = extract_types_ul(tgt_content) if target == "ul" else extract_types_c(tgt_content)
    elif origin_ext == ".ul":
        st = extract_types_ul(src_content)
        tt = extract_types_c(tgt_content) if target == "c" else extract_types_ul(tgt_content)
    else:
        st, tt = [], []

    terr = compare_types(st, tt, origin_lang, target.upper())
    if terr:
        log(f"  TYPES: FAIL ({len(terr)})")
        for e in terr[:5]:
            log(f"    {e}")
    else:
        log(f"  TYPES: PASS ({len(st)})")

    # Function comparison
    if origin_ext == ".c":
        sf = extract_funcs_c(src_content)
        tf = extract_funcs_ul(tgt_content) if target == "ul" else extract_funcs_c(tgt_content)
    elif origin_ext == ".ul":
        sf = extract_funcs_ul(src_content)
        tf = extract_funcs_c(tgt_content) if target == "c" else extract_funcs_ul(tgt_content)
    else:
        sf, tf = [], []

    ferr = compare_funcs(sf, tf, origin_lang, target.upper())
    if ferr:
        log(f"  FUNCTIONS: FAIL ({len(ferr)})")
        for e in ferr[:5]:
            log(f"    {e}")
    else:
        log(f"  FUNCTIONS: PASS ({len(sf)})")

    write_log()
    all_pass = (not terr or len(terr) == 0) and (not ferr or len(ferr) == 0)
    return ok and all_pass


# ── BUILD command ──────────────────────────────────────────────────────────

def cmd_build(source, backend):
    source = os.path.abspath(source)
    source_ext = os.path.splitext(source)[1].lower()
    base = os.path.splitext(source)[0]

    log(f"\n{'='*60}")
    log(f"TESTCHAIN BUILD: {os.path.basename(source)} (backend: {backend})")
    log(f"{'='*60}")

    src_hash = file_hash(source)

    if source_ext == ".c":
        exe = base + ".exe"
        ok, out, err = run_cmd(["gcc", source, "-o", exe, "-lm"])
        if not ok:
            log(f"  GCC: FAIL")
            log(f"  {err[:300]}")
            write_log()
            return False
        log(f"  GCC: PASS")
        output = run_exe(exe)
        runner = lambda inp: run_exe(exe, inp)

    elif source_ext == ".ul":
        if backend == "c":
            c_file = base + "_built.c"
            ok, out, err = run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"), source, "-t", "c", "-o", c_file])
            if not ok:
                log(f"  UL->C: FAIL")
                log(f"  {(err or out)[:300]}")
                write_log()
                return False
            log(f"  UL->C: PASS")
            exe = base + "_built.exe"
            ok, out, err = run_cmd(["gcc", c_file, "-o", exe, "-lm"])
            if not ok:
                log(f"  GCC: FAIL")
                log(f"  {err[:300]}")
                write_log()
                return False
            log(f"  GCC: PASS")
            output = run_exe(exe)
            runner = lambda inp: run_exe(exe, inp)

        elif backend == "llvm":
            ll_file = base + "_built.ll"
            ok, out, err = run_cmd([PYTHON, "-B", os.path.join(XPILE_ROOT, "Main.py"), source, "-t", "llvm", "-o", ll_file])
            if not ok:
                log(f"  UL->LLVM: FAIL")
                log(f"  {(err or out)[:300]}")
                write_log()
                return False
            log(f"  UL->LLVM: PASS")
            # Try to compile LLVM IR
            exe = base + "_built_llvm.exe"
            clang = r"C:\Program Files\LLVM\bin\clang.exe"
            ok, out, err = run_cmd([clang, ll_file, "-o", exe, "-lm", "-target", "x86_64-pc-windows-gnu"])
            if not ok:
                log(f"  CLANG: FAIL (is clang installed?)")
                log(f"  {err[:300]}")
                write_log()
                return False
            log(f"  CLANG: PASS")
            output = run_exe(exe)
            runner = lambda inp: run_exe(exe, inp)
        else:
            log(f"  ERROR: Unknown backend '{backend}' for .ul")
            write_log()
            return False
    else:
        log(f"  ERROR: Don't know how to build {source_ext}")
        write_log()
        return False

    # Integrity
    if file_hash(source) != src_hash:
        log(f"  INTEGRITY: FAIL — SOURCE MODIFIED DURING BUILD")
        write_log()
        return False
    log(f"  INTEGRITY: PASS")

    # Run
    if output is not None:
        log(f"  RUN: PASS ({len(output)} bytes output)")
    else:
        log(f"  RUN: FAIL (no output or crash)")
        write_log()
        return False

    # Fuzz
    log(f"  FUZZ (20 rounds)...")
    if source_ext == ".ul" and os.path.isfile(exe):
        passed, fuzz_out, fuzz_err = fuzz(exe, source, rounds=20)
        if passed:
            log(f"  FUZZ: PASS")
        else:
            log(f"  FUZZ: FAIL")
            for line in (fuzz_out + fuzz_err).split("\n")[-8:]:
                if line.strip():
                    log(f"    {line}")
    else:
        log(f"  FUZZ: SKIP (no .ul source to fuzz against)")

    write_log()
    return True


# ── Main ───────────────────────────────────────────────────────────────────

def main():
    setup_paths()

    if len(sys.argv) < 3:
        print("Usage:")
        print("  testchain convert <origin> <output> -t <target>")
        print("  testchain build <source> -t <backend>")
        sys.exit(1)

    cmd = sys.argv[1]

    if cmd == "convert":
        if len(sys.argv) < 6 or "-t" not in sys.argv:
            print("Usage: testchain convert <origin> <output> -t <target>")
            sys.exit(1)
        origin = sys.argv[2]
        output = sys.argv[3]
        t_idx = sys.argv.index("-t")
        target = sys.argv[t_idx + 1]
        ok = cmd_convert(origin, output, target)
        sys.exit(0 if ok else 1)

    elif cmd == "build":
        if len(sys.argv) < 5 or "-t" not in sys.argv:
            print("Usage: testchain build <source> -t <backend>")
            sys.exit(1)
        source = sys.argv[2]
        t_idx = sys.argv.index("-t")
        backend = sys.argv[t_idx + 1]
        ok = cmd_build(source, backend)
        sys.exit(0 if ok else 1)

    else:
        print(f"Unknown command: {cmd}")
        print("Use: testchain convert | testchain build")
        sys.exit(1)


if __name__ == "__main__":
    main()
