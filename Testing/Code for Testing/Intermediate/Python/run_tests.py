#!/usr/bin/env python3
"""Phase 3A Test Runner: py2ul transpiled programs — compile UL to Python and C, compare output."""

import os
import sys
import subprocess
import time
import shutil

PYTHON = sys.executable
XPILE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MAIN_PY = os.path.join(XPILE_DIR, "Main.py")
PHASE_DIR = os.path.dirname(os.path.abspath(__file__))

GCC = shutil.which("gcc") or "gcc"

TESTS = [
    "T300_fizzbuzz",
    "T301_calculator",
    "T302_stats",
    "T303_text_proc",
]


def run_target(test_name, target):
    """Compile and run a test for a given target (python or c). Returns (status, detail)."""
    test_dir = os.path.join(PHASE_DIR, test_name)
    ul_file = os.path.join(test_dir, "test.ul")
    expected_file = os.path.join(test_dir, "expected.txt")
    failure_file = os.path.join(test_dir, f"failure_{target}.txt")

    if os.path.isfile(failure_file):
        os.remove(failure_file)

    if target == "python":
        out_file = os.path.join(test_dir, "test.py")
        compile_result = subprocess.run(
            [PYTHON, MAIN_PY, ul_file, "-t", "python", "-o", out_file],
            capture_output=True, text=True, cwd=XPILE_DIR, timeout=30
        )
        if compile_result.returncode != 0:
            msg = f"UL->Python COMPILE FAILED:\n{compile_result.stdout}\n{compile_result.stderr}"
            with open(failure_file, "w") as f:
                f.write(msg)
            return "FAIL", f"UL compile error"

        try:
            run_result = subprocess.run(
                [PYTHON, out_file], capture_output=True, text=True, cwd=test_dir, timeout=30
            )
        except subprocess.TimeoutExpired:
            with open(failure_file, "w") as f:
                f.write("TIMEOUT")
            return "FAIL", "Runtime timeout"

    elif target == "c":
        c_file = os.path.join(test_dir, "test.c")
        exe_file = os.path.join(test_dir, "test.exe")
        compile_result = subprocess.run(
            [PYTHON, MAIN_PY, ul_file, "-t", "c", "-o", c_file],
            capture_output=True, text=True, cwd=XPILE_DIR, timeout=30
        )
        if compile_result.returncode != 0:
            msg = f"UL->C COMPILE FAILED:\n{compile_result.stdout}\n{compile_result.stderr}"
            with open(failure_file, "w") as f:
                f.write(msg)
            return "FAIL", f"UL compile error"

        gcc_result = subprocess.run(
            [GCC, c_file, "-o", exe_file, "-lm"],
            capture_output=True, text=True, cwd=test_dir, timeout=30
        )
        if gcc_result.returncode != 0:
            msg = f"GCC COMPILE FAILED:\n{gcc_result.stderr}"
            with open(failure_file, "w") as f:
                f.write(msg)
            return "FAIL", f"gcc error"

        try:
            run_result = subprocess.run(
                [exe_file], capture_output=True, text=True, cwd=test_dir, timeout=30
            )
        except subprocess.TimeoutExpired:
            with open(failure_file, "w") as f:
                f.write("TIMEOUT")
            return "FAIL", "Runtime timeout"

    actual_output = run_result.stdout
    with open(os.path.join(test_dir, f"actual_{target}.txt"), "w") as f:
        f.write(actual_output)

    with open(expected_file, "r") as f:
        expected = f.read()

    expected_lines = [l.rstrip() for l in expected.strip().splitlines()]
    actual_lines = [l.rstrip() for l in actual_output.strip().splitlines()]

    if expected_lines == actual_lines:
        return "PASS", None

    diff_lines = []
    max_lines = max(len(expected_lines), len(actual_lines))
    for i in range(max_lines):
        exp = expected_lines[i] if i < len(expected_lines) else "<MISSING>"
        act = actual_lines[i] if i < len(actual_lines) else "<MISSING>"
        marker = "  " if exp == act else "!!"
        diff_lines.append(f"{marker} line {i+1}: expected={repr(exp)}  actual={repr(act)}")

    msg = "OUTPUT MISMATCH:\n" + "\n".join(diff_lines)
    with open(failure_file, "w") as f:
        f.write(msg)
    return "FAIL", f"Output mismatch ({len([l for l in diff_lines if l.startswith('!!')])} lines differ)"


def main():
    print(f"Phase 3A Test Runner — {len(TESTS)} tests x 2 targets (Python, C)")
    print(f"Compiler: {MAIN_PY}")
    print(f"GCC: {GCC}")
    print("=" * 60)

    results = {}
    start = time.time()

    for test_name in TESTS:
        test_dir = os.path.join(PHASE_DIR, test_name)
        ul_file = os.path.join(test_dir, "test.ul")
        expected_file = os.path.join(test_dir, "expected.txt")

        if not os.path.isfile(ul_file) or not os.path.isfile(expected_file):
            results[test_name] = ("SKIP", "SKIP")
            print(f"  SKIP  {test_name}")
            continue

        py_status, py_detail = run_target(test_name, "python")
        c_status, c_detail = run_target(test_name, "c")
        results[test_name] = (py_status, c_status)

        py_line = f"Py:{py_status}"
        if py_detail:
            py_line += f"({py_detail})"
        c_line = f"C:{c_status}"
        if c_detail:
            c_line += f"({c_detail})"
        print(f"  {py_line:30s}  {c_line:30s}  {test_name}")

    elapsed = time.time() - start
    total_py = sum(1 for s in results.values() if s[0] == "PASS")
    total_c = sum(1 for s in results.values() if s[1] == "PASS")
    total = len(TESTS)

    print("=" * 60)
    print(f"Results: Python {total_py}/{total}, C {total_c}/{total}  ({elapsed:.1f}s)")

    failed = any(s[0] != "PASS" or s[1] != "PASS" for s in results.values())
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
