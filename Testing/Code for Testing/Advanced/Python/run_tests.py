"""Phase 3C test runner — py2ul complex Python scripts, dual target (Python + C)."""
import subprocess
import sys
import os

PYTHON = r"C:\Users\azt12\AppData\Local\Programs\Python\Python312\python.exe"
GCC = r"C:\Users\azt12\mingw64\bin\gcc.exe"
XPILE = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), "Main.py")

TESTS = [
    "T320_sorting",
    "T321_pipeline",
    "T322_statemachine",
]

def run(cmd, cwd=None, timeout=30):
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd, timeout=timeout)
    return r.returncode, r.stdout, r.stderr

def run_target(test_name, target):
    test_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), test_name)
    ul_file = os.path.join(test_dir, "test.ul")

    # Choose expected file
    if target == "c":
        exp_file = os.path.join(test_dir, "expected_c.txt")
        if not os.path.exists(exp_file):
            exp_file = os.path.join(test_dir, "expected.txt")
    else:
        exp_file = os.path.join(test_dir, "expected.txt")

    expected = open(exp_file).read().strip()

    # Compile UL
    rc, out, err = run([PYTHON, XPILE, ul_file, "-t", target])
    if rc != 0:
        return "FAIL", f"UL compile error: {err}"

    if target == "python":
        py_file = os.path.join(test_dir, "test.py")
        rc, out, err = run([PYTHON, py_file])
        if rc != 0:
            return "FAIL", f"Python runtime error: {err}"
    elif target == "c":
        c_file = os.path.join(test_dir, "test.c")
        exe_file = os.path.join(test_dir, "test.exe")
        rc, out, err = run([GCC, c_file, "-o", exe_file])
        if rc != 0:
            return "FAIL", f"GCC compile error: {err}"
        rc, out, err = run([exe_file])
        if rc != 0:
            return "FAIL", f"C runtime error: {err}"

    actual = out.strip()
    if actual == expected:
        return "PASS", ""
    else:
        return "FAIL", f"Output mismatch:\nEXPECTED:\n{expected}\nACTUAL:\n{actual}"

def main():
    results = []
    for test_name in TESTS:
        for target in ["python", "c"]:
            status, msg = run_target(test_name, target)
            label = f"{test_name} [{target}]"
            results.append((label, status, msg))
            icon = "PASS" if status == "PASS" else "FAIL"
            print(f"  {icon}  {label}")
            if msg:
                print(f"       {msg}")

    passed = sum(1 for _, s, _ in results if s == "PASS")
    total = len(results)
    print(f"\n{passed}/{total} passed")
    return 0 if passed == total else 1

if __name__ == "__main__":
    sys.exit(main())
