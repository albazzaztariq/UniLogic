#!/usr/bin/env python3
"""
Phase 5C: Cross-target proof
Verify that the SAME UL program produces IDENTICAL output when compiled to Python AND C.
"""

import os
import sys
import subprocess
import shutil
import tempfile

XPILE_DIR = os.path.join(os.path.dirname(__file__), "..", "..")
XPILE_DIR = os.path.normpath(XPILE_DIR)
MAIN_PY = os.path.join(XPILE_DIR, "Main.py")
PYTHON = sys.executable
GCC = shutil.which("gcc") or "gcc"

PHASE1A_DIR = os.path.join(XPILE_DIR, "Tests", "Phase1A")
MANAGER_DIR = os.path.join(XPILE_DIR, "Tests", "ManagerTests")
PHASE5C_DIR = os.path.dirname(os.path.abspath(__file__))

PHASE1A_TESTS = [
    ("T100", "T100_hello/test.ul"),
    ("T101", "T101_functions/test.ul"),
    ("T102", "T102_control_flow/test.ul"),
    ("T103", "T103_collections/test.ul"),
    ("T104", "T104_types/test.ul"),
    ("T105", "T105_objects/test.ul"),
    ("T106", "T106_error_handling/test.ul"),
    ("T107", "T107_file_io/test.ul"),
    ("T108", "T108_strings/test.ul"),
    ("T109", "T109_math/test.ul"),
    ("T110", "T110_concurrency/test.ul"),
    ("T111", "T111_dr/test.ul"),
    ("T112", "T112_enums/test.ul"),
    ("T113", "T113_nested_fns/test.ul"),
    ("T114", "T114_with/test.ul"),
    ("T115", "T115_pipe_bitwise/test.ul"),
]

MANAGER_TESTS = [
    ("M001", "M001_basics.ul"),
    ("M002", "M002_functions.ul"),
    ("M003", "M003_control_flow.ul"),
    ("M004", "M004_collections.ul"),
    ("M005", "M005_objects.ul"),
    ("M006", "M006_error_handling.ul"),
    ("M007", "M007_casting.ul"),
    ("M008", "M008_enums_constants.ul"),
]

def run_cmd(cmd, cwd=None, timeout=30):
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            cwd=cwd,
            timeout=timeout
        )
        return result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired:
        return "", "TIMEOUT", -1
    except Exception as e:
        return "", str(e), -1

def normalize_output(text):
    """Normalize line endings and strip trailing whitespace."""
    lines = text.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    lines = [l.rstrip() for l in lines]
    # Remove trailing empty lines
    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines)

def compile_to_python(ul_file, out_dir):
    """Compile .ul -> .py, return (py_file, stderr, returncode)"""
    base = os.path.splitext(os.path.basename(ul_file))[0]
    out_py = os.path.join(out_dir, base + ".py")
    stdout, stderr, rc = run_cmd(
        [PYTHON, MAIN_PY, ul_file, "-t", "python", "-o", out_py],
        cwd=XPILE_DIR
    )
    return out_py, stderr, rc

def run_python(py_file, cwd=None):
    """Run .py file, return (stdout, stderr, returncode)"""
    return run_cmd([PYTHON, py_file], cwd=cwd or os.path.dirname(py_file))

def compile_to_c(ul_file, out_dir):
    """Compile .ul -> .c, return (c_file, stderr, returncode)"""
    base = os.path.splitext(os.path.basename(ul_file))[0]
    out_c = os.path.join(out_dir, base + ".c")
    stdout, stderr, rc = run_cmd(
        [PYTHON, MAIN_PY, ul_file, "-t", "c", "-o", out_c],
        cwd=XPILE_DIR
    )
    return out_c, stderr, rc

def compile_c(c_file, out_dir):
    """Compile .c -> .exe with gcc, return (exe_file, stderr, returncode)"""
    base = os.path.splitext(os.path.basename(c_file))[0]
    out_exe = os.path.join(out_dir, base + ".exe")
    stdout, stderr, rc = run_cmd(
        [GCC, c_file, "-o", out_exe, "-lm"],
        cwd=out_dir
    )
    return out_exe, stderr, rc

def run_exe(exe_file, cwd=None):
    """Run executable, return (stdout, stderr, returncode)"""
    return run_cmd([exe_file], cwd=cwd or os.path.dirname(exe_file))

def run_single_test(test_id, ul_path, work_dir):
    """Run one cross-target test. Returns result dict."""
    result = {
        "id": test_id,
        "ul_file": ul_path,
        "status": "UNKNOWN",
        "py_compile_ok": False,
        "py_run_ok": False,
        "c_compile_ok": False,
        "gcc_compile_ok": False,
        "c_run_ok": False,
        "py_output": "",
        "c_output": "",
        "match": False,
        "notes": [],
    }

    os.makedirs(work_dir, exist_ok=True)

    # --- Python target ---
    print(f"  [PY compile] {test_id}...", end="", flush=True)
    py_file, py_stderr, py_rc = compile_to_python(ul_path, work_dir)
    if py_rc != 0:
        result["notes"].append(f"PY compile FAILED: {py_stderr.strip()[:200]}")
        result["status"] = "PY_COMPILE_FAIL"
        print(f" FAIL")
        return result
    result["py_compile_ok"] = True
    print(f" OK")

    print(f"  [PY run]     {test_id}...", end="", flush=True)
    py_stdout, py_run_err, py_run_rc = run_python(py_file)
    if py_run_rc != 0:
        result["notes"].append(f"PY run FAILED (rc={py_run_rc}): {py_run_err.strip()[:200]}")
        result["status"] = "PY_RUN_FAIL"
        result["py_output"] = py_stdout
        print(f" FAIL")
        return result
    result["py_run_ok"] = True
    result["py_output"] = normalize_output(py_stdout)
    print(f" OK")

    # --- C target ---
    print(f"  [C compile]  {test_id}...", end="", flush=True)
    c_file, c_stderr, c_rc = compile_to_c(ul_path, work_dir)
    if c_rc != 0:
        result["notes"].append(f"C compile FAILED: {c_stderr.strip()[:200]}")
        result["status"] = "C_COMPILE_FAIL"
        print(f" FAIL")
        return result
    result["c_compile_ok"] = True
    print(f" OK")

    print(f"  [GCC compile]{test_id}...", end="", flush=True)
    exe_file, gcc_stderr, gcc_rc = compile_c(c_file, work_dir)
    if gcc_rc != 0:
        result["notes"].append(f"GCC FAILED: {gcc_stderr.strip()[:300]}")
        result["status"] = "GCC_FAIL"
        print(f" FAIL")
        return result
    result["gcc_compile_ok"] = True
    print(f" OK")

    print(f"  [C run]      {test_id}...", end="", flush=True)
    c_stdout, c_run_err, c_run_rc = run_exe(exe_file)
    if c_run_rc != 0:
        result["notes"].append(f"C run FAILED (rc={c_run_rc}): {c_run_err.strip()[:200]}")
        result["status"] = "C_RUN_FAIL"
        result["c_output"] = c_stdout
        print(f" FAIL")
        return result
    result["c_run_ok"] = True
    result["c_output"] = normalize_output(c_stdout)
    print(f" OK")

    # --- Compare ---
    if result["py_output"] == result["c_output"]:
        result["match"] = True
        result["status"] = "PASS"
    else:
        result["match"] = False
        result["status"] = "MISMATCH"
        result["notes"].append("OUTPUT MISMATCH between Python and C targets")

    return result

def diff_outputs(py_out, c_out):
    """Show line-by-line diff."""
    py_lines = py_out.split("\n")
    c_lines = c_out.split("\n")
    max_len = max(len(py_lines), len(c_lines))
    lines = []
    for i in range(max_len):
        py_l = py_lines[i] if i < len(py_lines) else "<missing>"
        c_l = c_lines[i] if i < len(c_lines) else "<missing>"
        if py_l != c_l:
            lines.append(f"  Line {i+1}: PY={repr(py_l)}  C={repr(c_l)}")
    return "\n".join(lines)

def main():
    print("=" * 60)
    print("Phase 5C: Cross-Target Proof (Python vs C output)")
    print("=" * 60)
    print(f"XPile dir: {XPILE_DIR}")
    print(f"GCC: {GCC}")
    print()

    all_tests = []

    # Phase1A tests
    print("[Phase1A Tests: T100-T115]")
    print("-" * 40)
    for test_id, rel_ul in PHASE1A_TESTS:
        ul_path = os.path.join(PHASE1A_DIR, rel_ul)
        if not os.path.exists(ul_path):
            print(f"  SKIP {test_id}: ul file not found")
            continue
        work_dir = os.path.join(PHASE5C_DIR, "work", test_id)
        print(f"\n{test_id}:")
        r = run_single_test(test_id, ul_path, work_dir)
        all_tests.append(r)

    # ManagerTests M001-M008
    print()
    print("[ManagerTests: M001-M008]")
    print("-" * 40)
    for test_id, ul_name in MANAGER_TESTS:
        ul_path = os.path.join(MANAGER_DIR, ul_name)
        if not os.path.exists(ul_path):
            print(f"  SKIP {test_id}: ul file not found")
            continue
        work_dir = os.path.join(PHASE5C_DIR, "work", test_id)
        print(f"\n{test_id}:")
        r = run_single_test(test_id, ul_path, work_dir)
        all_tests.append(r)

    # Summary
    print()
    print("=" * 60)
    print("SUMMARY")
    print("=" * 60)

    passed = [r for r in all_tests if r["status"] == "PASS"]
    mismatches = [r for r in all_tests if r["status"] == "MISMATCH"]
    failures = [r for r in all_tests if r["status"] not in ("PASS", "MISMATCH")]

    print(f"Total tests: {len(all_tests)}")
    print(f"PASS (identical output): {len(passed)}")
    print(f"MISMATCH (compiler bug): {len(mismatches)}")
    print(f"Other failures: {len(failures)}")
    print()

    if mismatches:
        print("--- MISMATCHES (Output differs between Python and C) ---")
        for r in mismatches:
            print(f"\n{r['id']} MISMATCH:")
            print(f"  PY output:\n    " + r["py_output"].replace("\n", "\n    "))
            print(f"  C output:\n    " + r["c_output"].replace("\n", "\n    "))
            print(f"  Diff:")
            print(diff_outputs(r["py_output"], r["c_output"]))

    if failures:
        print()
        print("--- OTHER FAILURES ---")
        for r in failures:
            print(f"\n{r['id']} {r['status']}:")
            for note in r["notes"]:
                print(f"  {note}")

    print()
    print("--- PASSED ---")
    for r in passed:
        print(f"  {r['id']} PASS")

    # Build RESULTS.md content
    results_md = build_results_md(all_tests, passed, mismatches, failures)
    results_path = os.path.join(PHASE5C_DIR, "RESULTS.md")
    with open(results_path, "w") as f:
        f.write(results_md)
    print(f"\nResults written to: {results_path}")

    return 0 if not mismatches and not failures else 1

def build_results_md(all_tests, passed, mismatches, failures):
    lines = []
    lines.append("# Phase 5C: Cross-Target Proof Results")
    lines.append("")
    lines.append("Verifies that the same UL program produces **identical output** when compiled to Python and C.")
    lines.append("")
    lines.append(f"**Total tests:** {len(all_tests)}")
    lines.append(f"**PASS (identical):** {len(passed)}")
    lines.append(f"**MISMATCH (compiler bug):** {len(mismatches)}")
    lines.append(f"**Other failures:** {len(failures)}")
    lines.append("")

    lines.append("## Results Table")
    lines.append("")
    lines.append("| Test | PY Compile | PY Run | C Compile | GCC | C Run | Match | Status |")
    lines.append("|------|-----------|--------|-----------|-----|-------|-------|--------|")
    for r in all_tests:
        def yn(b): return "YES" if b else "NO"
        match_str = "IDENTICAL" if r["match"] else ("MISMATCH" if r["c_run_ok"] and r["py_run_ok"] else "N/A")
        lines.append(
            f"| {r['id']} | {yn(r['py_compile_ok'])} | {yn(r['py_run_ok'])} | "
            f"{yn(r['c_compile_ok'])} | {yn(r['gcc_compile_ok'])} | {yn(r['c_run_ok'])} | "
            f"{match_str} | {r['status']} |"
        )
    lines.append("")

    if mismatches:
        lines.append("## Mismatches (Compiler Bugs)")
        lines.append("")
        lines.append("These are real bugs — same UL program produces different output on different targets.")
        lines.append("")
        for r in mismatches:
            lines.append(f"### {r['id']}")
            lines.append("")
            lines.append("**Python output:**")
            lines.append("```")
            lines.append(r["py_output"])
            lines.append("```")
            lines.append("")
            lines.append("**C output:**")
            lines.append("```")
            lines.append(r["c_output"])
            lines.append("```")
            lines.append("")
            lines.append("**Diff:**")
            lines.append("```")
            lines.append(diff_outputs(r["py_output"], r["c_output"]))
            lines.append("```")
            lines.append("")

    if failures:
        lines.append("## Other Failures")
        lines.append("")
        for r in failures:
            lines.append(f"### {r['id']} — {r['status']}")
            for note in r["notes"]:
                lines.append(f"- {note}")
            lines.append("")

    if passed:
        lines.append("## Verified Passing Tests")
        lines.append("")
        lines.append("These programs produce identical output on both Python and C targets:")
        lines.append("")
        for r in passed:
            lines.append(f"- **{r['id']}** — output matches exactly")
        lines.append("")

    return "\n".join(lines)


if __name__ == "__main__":
    sys.exit(main())
