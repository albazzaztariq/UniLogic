#!/usr/bin/env python3
"""Phase 1A Test Runner: Compile UL->Python, run, compare with expected output."""

import os
import sys
import subprocess
import time

PYTHON = sys.executable
XPILE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MAIN_PY = os.path.join(XPILE_DIR, "Main.py")
PHASE_DIR = os.path.dirname(os.path.abspath(__file__))

TESTS = [
    "T100_hello",
    "T101_functions",
    "T102_control_flow",
    "T103_collections",
    "T104_types",
    "T105_objects",
    "T106_error_handling",
    "T107_file_io",
    "T108_strings",
    "T109_math",
    "T110_concurrency",
    "T111_dr",
    "T112_enums",
    "T113_nested_fns",
    "T114_with",
    "T115_pipe_bitwise",
]

def run_test(test_name):
    test_dir = os.path.join(PHASE_DIR, test_name)
    ul_file = os.path.join(test_dir, "test.ul")
    expected_file = os.path.join(test_dir, "expected.txt")
    py_output = os.path.join(test_dir, "test.py")
    actual_file = os.path.join(test_dir, "actual.txt")
    failure_file = os.path.join(test_dir, "failure.txt")

    if not os.path.isfile(ul_file):
        return "SKIP", f"No test.ul found in {test_name}"
    if not os.path.isfile(expected_file):
        return "SKIP", f"No expected.txt found in {test_name}"

    # Clean previous failure log
    if os.path.isfile(failure_file):
        os.remove(failure_file)

    # Step 1: Compile UL -> Python
    compile_result = subprocess.run(
        [PYTHON, MAIN_PY, ul_file, "-t", "python", "-o", py_output],
        capture_output=True, text=True, cwd=XPILE_DIR, timeout=30
    )
    if compile_result.returncode != 0:
        msg = f"COMPILE FAILED:\nstdout: {compile_result.stdout}\nstderr: {compile_result.stderr}"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", f"Compilation error: {compile_result.stderr.strip()[:200]}"

    # Step 2: Run the generated Python
    try:
        run_result = subprocess.run(
            [PYTHON, py_output],
            capture_output=True, text=True, cwd=test_dir, timeout=30
        )
    except subprocess.TimeoutExpired:
        msg = "RUNTIME TIMEOUT: Python execution exceeded 30 seconds"
        with open(failure_file, "w") as f:
            f.write(msg)
        return "FAIL", "Runtime timeout"

    actual_output = run_result.stdout
    if run_result.returncode != 0 and run_result.stderr:
        # Some tests may have non-zero exit but still produce output
        # Only fail if there's no stdout at all
        if not actual_output.strip():
            msg = f"RUNTIME ERROR (exit {run_result.returncode}):\nstderr: {run_result.stderr}"
            with open(failure_file, "w") as f:
                f.write(msg)
            return "FAIL", f"Runtime error: {run_result.stderr.strip()[:200]}"

    # Save actual output
    with open(actual_file, "w") as f:
        f.write(actual_output)

    # Step 3: Compare
    with open(expected_file, "r") as f:
        expected = f.read()

    # Normalize: strip trailing whitespace from each line and trailing newlines
    expected_lines = [l.rstrip() for l in expected.strip().splitlines()]
    actual_lines = [l.rstrip() for l in actual_output.strip().splitlines()]

    if expected_lines == actual_lines:
        return "PASS", None
    else:
        # Build diff report
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
    print(f"Phase 1A Test Runner — {len(TESTS)} tests")
    print(f"Compiler: {MAIN_PY}")
    print(f"Python: {PYTHON}")
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
