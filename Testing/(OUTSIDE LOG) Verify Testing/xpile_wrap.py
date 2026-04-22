#!/usr/bin/env python3
"""xpile_wrap.py — Transpilation wrapper that logs every build attempt.

Usage:
    python xpile_wrap.py <origin_file> <new_file> <target>

Arguments:
    origin_file: source file to compile (test.c, test.ul, etc)
    new_file:    output file to create (test.ul, test_rt.c, etc)
    target:      target language (ul, c, python, js)

Every invocation:
    1. Runs the appropriate compiler
    2. Copies all 3 files (compiler script, origin, target) to VerifyTest/<library>/
    3. Logs SUCCESS or FAILURE with timestamp to dashboard
"""

import sys
import os
import shutil
import subprocess
import json
import time
from datetime import datetime

XPILE_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VERIFY_DIR = os.path.join(XPILE_ROOT, "VerifyTest")
DASHBOARD_FILE = os.path.join(VERIFY_DIR, "dashboard.jsonl")
PYTHON = sys.executable

def get_library_name(origin_file):
    """Extract library/test name from the origin file path."""
    base = os.path.basename(origin_file)
    name = os.path.splitext(base)[0]
    # Strip common prefixes
    for prefix in ("test_", "test-"):
        if name.startswith(prefix):
            name = name[len(prefix):]
            break
    return name

def detect_compiler_and_command(origin_file, new_file, target):
    """Determine which compiler script to use and build the command."""
    origin_ext = os.path.splitext(origin_file)[1].lower()

    if target == "ul":
        if origin_ext == ".c":
            script = os.path.join(XPILE_ROOT, "Tools", "c2ul.py")
            cmd = [PYTHON, "-B", script, origin_file, "-o", new_file]
            return script, cmd, "c2ul"
        elif origin_ext == ".py":
            script = os.path.join(XPILE_ROOT, "Tools", "py2ul.py")
            cmd = [PYTHON, "-B", script, origin_file, "-o", new_file]
            return script, cmd, "py2ul"
    elif target == "c":
        if origin_ext == ".ul":
            script = os.path.join(XPILE_ROOT, "Main.py")
            cmd = [PYTHON, "-B", script, origin_file, "-t", "c", "-o", new_file]
            return script, cmd, "codegen_c"
    elif target == "python":
        if origin_ext == ".ul":
            script = os.path.join(XPILE_ROOT, "Main.py")
            cmd = [PYTHON, "-B", script, origin_file, "-t", "python", "-o", new_file]
            return script, cmd, "codegen_python"
    elif target == "js":
        if origin_ext == ".ul":
            script = os.path.join(XPILE_ROOT, "Main.py")
            cmd = [PYTHON, "-B", script, origin_file, "-t", "js", "-o", new_file]
            return script, cmd, "codegen_js"
    elif target == "native":
        # gcc compile
        if origin_ext == ".c":
            script = "gcc"
            cmd = ["gcc", origin_file, "-o", new_file, "-lm"]
            return script, cmd, "gcc"

    return None, None, None

def copy_to_verify(library_name, timestamp_str, compiler_script, origin_file, new_file, success):
    """Copy all involved files to VerifyTest/<library>/."""
    lib_dir = os.path.join(VERIFY_DIR, library_name)
    os.makedirs(lib_dir, exist_ok=True)

    ts = timestamp_str.replace(":", "-").replace(" ", "_")

    # Copy compiler script
    if compiler_script and os.path.isfile(compiler_script):
        ext = os.path.splitext(compiler_script)[1]
        dest = os.path.join(lib_dir, f"{ts}_compiler{ext}")
        shutil.copy2(compiler_script, dest)

    # Copy origin file (skip binaries — can't diff them)
    if os.path.isfile(origin_file):
        ext = os.path.splitext(origin_file)[1]
        if ext.lower() not in (".exe", ".o", ".obj", ".dll", ".so", ".a"):
            dest = os.path.join(lib_dir, f"{ts}_origin{ext}")
            shutil.copy2(origin_file, dest)

    # Copy target file (skip binaries)
    if new_file and os.path.isfile(new_file):
        ext = os.path.splitext(new_file)[1]
        if ext.lower() not in (".exe", ".o", ".obj", ".dll", ".so", ".a"):
            dest = os.path.join(lib_dir, f"{ts}_target{ext}")
            shutil.copy2(new_file, dest)

def log_dashboard(entry):
    """Append entry to dashboard JSONL file."""
    os.makedirs(VERIFY_DIR, exist_ok=True)
    with open(DASHBOARD_FILE, "a", encoding="utf-8") as f:
        f.write(json.dumps(entry) + "\n")

def main():
    if len(sys.argv) != 4:
        print("Usage: python xpile_wrap.py <origin_file> <new_file> <target>")
        print("  target: ul, c, python, js, native")
        sys.exit(1)

    origin_file = os.path.abspath(sys.argv[1])
    new_file = os.path.abspath(sys.argv[2])
    target = sys.argv[3].lower()

    library_name = get_library_name(origin_file)
    timestamp = datetime.now()
    timestamp_str = timestamp.strftime("%Y%m%d_%H%M%S")

    # Detect compiler and build command
    compiler_script, cmd, compiler_name = detect_compiler_and_command(
        origin_file, new_file, target
    )

    if cmd is None:
        entry = {
            "timestamp": timestamp.isoformat(),
            "library": library_name,
            "origin": os.path.basename(origin_file),
            "target_file": os.path.basename(new_file),
            "target_lang": target,
            "compiler": "UNKNOWN",
            "status": "FAILURE",
            "error": f"Cannot determine compiler for {os.path.splitext(origin_file)[1]} -> {target}",
        }
        log_dashboard(entry)
        print(f"[{timestamp_str}] FAILURE: Cannot determine compiler for {origin_file} -> {target}")
        sys.exit(1)

    # Run the compile
    print(f"[{timestamp_str}] BUILD: {os.path.basename(origin_file)} -> {os.path.basename(new_file)} ({compiler_name})")

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=120,
            cwd=XPILE_ROOT,
        )

        success = result.returncode == 0
        stdout = result.stdout.strip()
        stderr = result.stderr.strip()
        output = stdout if stdout else stderr

        status = "SUCCESS" if success else "FAILURE"

        entry = {
            "timestamp": timestamp.isoformat(),
            "library": library_name,
            "origin": os.path.basename(origin_file),
            "target_file": os.path.basename(new_file),
            "target_lang": target,
            "compiler": compiler_name,
            "status": status,
            "output": output[:500] if output else "",
            "return_code": result.returncode,
        }

        log_dashboard(entry)

        # Copy files — always, pass or fail
        copy_to_verify(library_name, timestamp_str, compiler_script, origin_file, new_file, success)

        # Print result
        print(f"[{timestamp_str}] {status}: {compiler_name} ({os.path.basename(origin_file)} -> {os.path.basename(new_file)})")
        if output:
            # Print compiler output (warnings, errors, etc)
            for line in output.split("\n")[:10]:
                print(f"  {line}")

        sys.exit(0 if success else 1)

    except subprocess.TimeoutExpired:
        entry = {
            "timestamp": timestamp.isoformat(),
            "library": library_name,
            "origin": os.path.basename(origin_file),
            "target_file": os.path.basename(new_file),
            "target_lang": target,
            "compiler": compiler_name,
            "status": "FAILURE",
            "error": "TIMEOUT (120s)",
        }
        log_dashboard(entry)
        copy_to_verify(library_name, timestamp_str, compiler_script, origin_file, new_file, False)
        print(f"[{timestamp_str}] FAILURE: TIMEOUT after 120s")
        sys.exit(1)
    except Exception as e:
        entry = {
            "timestamp": timestamp.isoformat(),
            "library": library_name,
            "origin": os.path.basename(origin_file),
            "target_file": os.path.basename(new_file),
            "target_lang": target,
            "compiler": compiler_name,
            "status": "FAILURE",
            "error": str(e),
        }
        log_dashboard(entry)
        copy_to_verify(library_name, timestamp_str, compiler_script, origin_file, new_file, False)
        print(f"[{timestamp_str}] FAILURE: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
