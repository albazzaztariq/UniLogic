#!/usr/bin/env python3
"""Phase 2B Test Runner: Compile UL->C->binary (multi-file), run, compare with expected output."""

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
    "T220_two_file",
    "T221_three_file",
    "T222_stdlib",
    "T223_http",
    "T224_cli_tool",
    "T225_game",
]

def run_test(test_name):
    test_dir = os.path.join(PHASE_DIR, test_name)
    ul_file = os.path.join(test_dir, "test.ul")
    expected_file = os.path.join(test_dir, "expected.txt")
    c_output = os.path.join(test_dir, "test.c")
    exe_output = os.path.join(test_dir, "test.exe")
    actual_file = os.path.join(test_dir, "actual.txt")
    failure_file = os.path.join(test_dir, "failure.txt")

    if not os.path.isfile(ul_file):
        return "SKIP", f"No test.ul found in {test_name}"
    if not os.path.isfile(expected_file):
        return "SKIP", f"No expected.txt found in {test_name}"

    if os.path.isfile(failure_file):
        os.remove(failure_file)

    # Step 1: Compile UL -> C
    compile_result = subprocess.run(
        [PYTHON, MAIN_PY, ul_file, "-t", "c", "-o", c_output],
        capture_output=True, text=True, cwd=XPILE_DIR, timeout=30
    )
    if compile_result.returncode != 0:
        msg = f"UL->C COMPILE FAILED:\nstdout: {compile_result.stdout}\nstderr: {compile_result.stderr}"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", f"UL compile error: {compile_result.stdout.strip()[:200]}"

    # Step 2: Compile C -> binary with gcc
    gcc_result = subprocess.run(
        [GCC, c_output, "-o", exe_output, "-lm"],
        capture_output=True, text=True, cwd=test_dir, timeout=30
    )
    if gcc_result.returncode != 0:
        msg = f"GCC COMPILE FAILED:\nstdout: {gcc_result.stdout}\nstderr: {gcc_result.stderr}"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", f"gcc error: {gcc_result.stderr.strip()[:200]}"

    # Step 3: Run the binary
    try:
        run_result = subprocess.run(
            [exe_output],
            capture_output=True, text=True, cwd=test_dir, timeout=30
        )
    except subprocess.TimeoutExpired:
        msg = "RUNTIME TIMEOUT: execution exceeded 30 seconds"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", "Runtime timeout"

    actual_output = run_result.stdout
    if run_result.returncode != 0 and not actual_output.strip():
        msg = f"RUNTIME ERROR (exit {run_result.returncode}):\nstderr: {run_result.stderr}"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", f"Runtime error: {run_result.stderr.strip()[:200]}"

    with open(actual_file, "w") as f:
        f.write(actual_output)

    # Step 4: Compare
    with open(expected_file, "r") as f:
        expected = f.read()

    expected_lines = [l.rstrip() for l in expected.strip().splitlines()]
    actual_lines = [l.rstrip() for l in actual_output.strip().splitlines()]

    if expected_lines == actual_lines:
        return "PASS", None
    else:
        diff_lines = []
        max_lines = max(len(expected_lines), len(actual_lines))
        for i in range(max_lines):
            exp = expected_lines[i] if i < len(expected_lines) else "<MISSING>"
            act = actual_lines[i] if i < len(actual_lines) else "<MISSING>"
            marker = "  " if exp == act else "!!"
            diff_lines.append(f"{marker} line {i+1}: expected={repr(exp)}  actual={repr(act)}")

        msg = "OUTPUT MISMATCH:\n" + "\n".join(diff_lines)
        if run_result.stderr:
            msg += f"\n\nstderr: {run_result.stderr}"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", f"Output mismatch ({len([l for l in diff_lines if l.startswith('!!')])} lines differ)"


def main():
    print(f"Phase 2B Test Runner — {len(TESTS)} tests")
    print(f"Compiler: {MAIN_PY}")
    print(f"GCC: {GCC}")
    print("=" * 60)

    results = {}
    start = time.time()

    for test_name in TESTS:
        status, detail = run_test(test_name)
        results[test_name] = status
        icon = {"PASS": "PASS", "FAIL": "FAIL", "SKIP": "SKIP"}[status]
        line = f"  {icon}  {test_name}"
        if detail:
            line += f"  -- {detail}"
        print(line)

    elapsed = time.time() - start
    passed = sum(1 for s in results.values() if s == "PASS")
    failed = sum(1 for s in results.values() if s == "FAIL")
    skipped = sum(1 for s in results.values() if s == "SKIP")

    print("=" * 60)
    print(f"Results: {passed}/{len(TESTS)} PASS, {failed} FAIL, {skipped} SKIP  ({elapsed:.1f}s)")

    if failed > 0:
        print("\nFailed tests (see failure.txt in each test folder for details):")
        for name, status in results.items():
            if status == "FAIL":
                print(f"  - {name}")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
