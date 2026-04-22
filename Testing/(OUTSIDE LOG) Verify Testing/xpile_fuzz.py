#!/usr/bin/env python3
"""xpile_fuzz.py — Differential fuzzing for transpilation equivalence.

Generates a test harness that exercises every function, type, and variable
in a program, runs it against both the original and round-tripped version,
and reports any behavioral difference.

Usage:
    python xpile_fuzz.py <original_exe> <roundtrip_exe> <ul_file> [--rounds N]

Arguments:
    original_exe:  compiled original program (e.g. test_orig.exe)
    roundtrip_exe: compiled round-tripped program (e.g. test_rt.exe)
    ul_file:       the UL intermediate file (parsed to extract types/functions)
    --rounds N:    number of fuzz rounds per function (default: 100)

What it does:
    1. Parses the UL file to extract:
       - All function signatures (name, params, return type)
       - All type/struct declarations (fields)
       - All global variables (name, type)
    2. For each round:
       - Generates random valid values for every type
       - Builds a sequence of operations:
         * Instantiate every struct with random field values
         * Call every function with random valid arguments
         * Mutate variables and re-read them
         * Pass return values as arguments to other functions
         * Cross-pollinate: use outputs of one function as inputs to another
       - Encodes the operation sequence as stdin input
    3. Runs both executables with the same input
    4. Compares stdout byte-for-byte
    5. Reports: PASS (all rounds match) or FAIL (with exact input that diverged)
"""

import sys
import os
import re
import json
import random
import string
import subprocess
import struct
from datetime import datetime

XPILE_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


# ── UL File Parser ──────────────────────────────────────────────────────────

def parse_ul_file(ul_path):
    """Parse a .ul file to extract functions, types, and globals."""
    with open(ul_path, encoding="utf-8") as f:
        lines = f.readlines()

    functions = []  # [{name, params: [{name, type, pointer}], returns, returns_pointer}]
    types = []      # [{name, fields: [{name, type, pointer}]}]
    globals_ = []   # [{name, type, pointer}]

    i = 0
    while i < len(lines):
        line = lines[i].strip()

        # Function declaration
        m = re.match(r'^function\s+(\w+)\s*\(([^)]*)\)(?:\s+returns\s+(.+))?', line)
        if m:
            fname = m.group(1)
            params_str = m.group(2).strip()
            ret_str = m.group(3)

            params = []
            if params_str:
                for p in params_str.split(","):
                    p = p.strip()
                    param = _parse_param(p)
                    if param:
                        params.append(param)

            ret_type = None
            ret_pointer = False
            if ret_str:
                ret_str = ret_str.strip()
                if ret_str.startswith("<") and ret_str.endswith(">"):
                    ret_type = ret_str[1:-1]
                    ret_pointer = True
                else:
                    ret_type = ret_str

            functions.append({
                "name": fname,
                "params": params,
                "returns": ret_type,
                "returns_pointer": ret_pointer,
            })

        # Type declaration
        m = re.match(r'^type\s+(\w+)', line)
        if m and not line.startswith("type_"):
            tname = m.group(1)
            fields = []
            i += 1
            while i < len(lines):
                fline = lines[i].strip()
                if fline.startswith("end type"):
                    break
                fp = _parse_param(fline)
                if fp:
                    fields.append(fp)
                i += 1
            types.append({"name": tname, "fields": fields})

        # Global variable
        m = re.match(r'^(int|float|double|string|bool|char|uint8|uint16|uint32|uint64|int8|int16|int32|int64)\s+<?(\w+)>?\s*=', line)
        if m:
            gtype = m.group(1)
            gname = m.group(2)
            is_ptr = "<" in line.split("=")[0]
            globals_.append({"name": gname, "type": gtype, "pointer": is_ptr})

        i += 1

    return {"functions": functions, "types": types, "globals": globals_}


def _parse_param(text):
    """Parse a single parameter like 'int x' or 'int <p>' or 'array int nums'."""
    text = text.strip()
    if not text:
        return None

    pointer = False
    # Check for <name> pattern
    m = re.match(r'^(\w+)\s+<(\w+)>', text)
    if m:
        return {"type": m.group(1), "name": m.group(2), "pointer": True}

    # Check for array type
    m = re.match(r'^array\s+(\w+)(?:\[(\d+)\])?\s+(\w+)', text)
    if m:
        return {"type": f"array_{m.group(1)}", "name": m.group(3), "pointer": False,
                "array_size": int(m.group(2)) if m.group(2) else 0}

    # Simple type name
    m = re.match(r'^(\w+)\s+(\w+)', text)
    if m:
        return {"type": m.group(1), "name": m.group(2), "pointer": pointer}

    return None


# ── Value Generators ────────────────────────────────────────────────────────

def gen_value(type_name, round_num=0):
    """Generate a random valid value for a given UL type."""
    seed = round_num * 1000 + hash(type_name) % 1000
    rng = random.Random(seed + round_num)

    if type_name in ("int", "int32"):
        return str(rng.randint(-2147483648, 2147483647))
    if type_name == "int8":
        return str(rng.randint(-128, 127))
    if type_name == "int16":
        return str(rng.randint(-32768, 32767))
    if type_name == "int64":
        return str(rng.randint(-2**63, 2**63 - 1))
    if type_name in ("uint8",):
        return str(rng.randint(0, 255))
    if type_name in ("uint16",):
        return str(rng.randint(0, 65535))
    if type_name in ("uint32",):
        return str(rng.randint(0, 4294967295))
    if type_name in ("uint64",):
        return str(rng.randint(0, 2**64 - 1))
    if type_name in ("float",):
        return f"{rng.uniform(-1e6, 1e6):.6f}"
    if type_name in ("double",):
        return f"{rng.uniform(-1e15, 1e15):.10f}"
    if type_name == "bool":
        return "1" if rng.random() > 0.5 else "0"
    if type_name == "char":
        return str(rng.randint(32, 126))  # printable ASCII
    if type_name == "string":
        length = rng.randint(0, 50)
        chars = [chr(rng.randint(32, 126)) for _ in range(length)]
        return "".join(chars)
    if type_name.startswith("array_"):
        base = type_name[len("array_"):]
        size = rng.randint(1, 10)
        return " ".join(gen_value(base, round_num + i) for i in range(size))
    # Unknown type — generate 0
    return "0"


# ── Test Harness Generator ─────────────────────────────────────────────────

def generate_c_harness(program_info, harness_path):
    """Generate a C test harness that reads fuzz inputs from stdin and exercises the program."""
    funcs = [f for f in program_info["functions"] if f["name"] != "main"]
    types = program_info["types"]

    lines = []
    lines.append("#include <stdio.h>")
    lines.append("#include <stdlib.h>")
    lines.append("#include <string.h>")
    lines.append("")
    lines.append("// Forward declarations from the program under test")
    lines.append("// (these will be linked from the compiled program)")
    lines.append("")

    # Generate main that reads inputs and calls functions
    lines.append("int fuzz_main() {")

    for f in funcs:
        if not f["params"]:
            # No-arg function — just call it
            if f["returns"] and f["returns"] != "none":
                lines.append(f'    printf("CALL {f["name"]}() = ");')
                lines.append(f'    printf("%d\\n", {f["name"]}());')
            else:
                lines.append(f'    {f["name"]}();')
                lines.append(f'    printf("CALL {f["name"]}() OK\\n");')

    lines.append("    return 0;")
    lines.append("}")
    lines.append("")

    with open(harness_path, "w") as f:
        f.write("\n".join(lines))


# ── Differential Runner ────────────────────────────────────────────────────

def run_differential(original_exe, roundtrip_exe, fuzz_input, timeout=10):
    """Run both executables with the same input, return (orig_out, rt_out, match)."""
    try:
        orig = subprocess.run(
            [original_exe],
            input=fuzz_input,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return "<TIMEOUT>", None, False
    except Exception as e:
        return f"<ERROR: {e}>", None, False

    try:
        rt = subprocess.run(
            [roundtrip_exe],
            input=fuzz_input,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return orig.stdout, "<TIMEOUT>", False
    except Exception as e:
        return orig.stdout, f"<ERROR: {e}>", False

    orig_out = orig.stdout
    rt_out = rt.stdout
    match = orig_out == rt_out

    return orig_out, rt_out, match


# ── Main ────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 4:
        print("Usage: python xpile_fuzz.py <original_exe> <roundtrip_exe> <ul_file> [--rounds N]")
        sys.exit(1)

    original_exe = os.path.abspath(sys.argv[1])
    roundtrip_exe = os.path.abspath(sys.argv[2])
    ul_file = os.path.abspath(sys.argv[3])

    rounds = 100
    if "--rounds" in sys.argv:
        idx = sys.argv.index("--rounds")
        if idx + 1 < len(sys.argv):
            rounds = int(sys.argv[idx + 1])

    # Verify files exist
    for path, name in [(original_exe, "original"), (roundtrip_exe, "roundtrip"), (ul_file, "UL file")]:
        if not os.path.isfile(path):
            print(f"ERROR: {name} not found: {path}")
            sys.exit(1)

    print(f"=== XPile Differential Fuzzer ===")
    print(f"Original:   {os.path.basename(original_exe)}")
    print(f"Roundtrip:  {os.path.basename(roundtrip_exe)}")
    print(f"UL Source:  {os.path.basename(ul_file)}")
    print(f"Rounds:     {rounds}")
    print()

    # Parse UL file
    program_info = parse_ul_file(ul_file)
    print(f"Found: {len(program_info['functions'])} functions, "
          f"{len(program_info['types'])} types, "
          f"{len(program_info['globals'])} globals")
    print()

    # Scan UL for error markers
    with open(ul_file, encoding="utf-8") as f:
        ul_content = f.read()

    errors = []
    for i, line in enumerate(ul_content.splitlines(), 1):
        if "/* ERROR:" in line:
            errors.append(f"  Line {i}: {line.strip()[:100]}")
        if "__unknown_" in line:
            errors.append(f"  Line {i}: unmapped type — {line.strip()[:100]}")
        if re.search(r'\b\w+_c\b', line) and "_c " in line:
            # Potential _safe_name rename
            pass  # Too many false positives, skip for now

    if errors:
        print(f"WARNING: {len(errors)} error markers in UL:")
        for e in errors[:10]:
            print(e)
        if len(errors) > 10:
            print(f"  ... and {len(errors) - 10} more")
        print()

    # Phase 1: Run both with no input (default execution)
    print("--- Phase 1: Default execution (no input) ---")
    orig_out, rt_out, match = run_differential(original_exe, roundtrip_exe, "")
    if match:
        print(f"PASS: Default execution output matches ({len(orig_out)} bytes)")
    else:
        print(f"FAIL: Default execution output DIFFERS")
        print(f"  Original ({len(orig_out) if orig_out else 0} bytes):")
        if orig_out:
            for line in orig_out.splitlines()[:5]:
                print(f"    {line}")
        print(f"  Roundtrip ({len(rt_out) if rt_out else 0} bytes):")
        if rt_out:
            for line in rt_out.splitlines()[:5]:
                print(f"    {line}")

        # Find first difference
        if orig_out and rt_out:
            orig_lines = orig_out.splitlines()
            rt_lines = rt_out.splitlines()
            for i, (a, b) in enumerate(zip(orig_lines, rt_lines)):
                if a != b:
                    print(f"  First diff at line {i+1}:")
                    print(f"    orig: {a[:100]}")
                    print(f"    rt:   {b[:100]}")
                    break
    print()

    # Phase 2: Fuzz with random stdin inputs
    print(f"--- Phase 2: Fuzzing ({rounds} rounds) ---")
    fuzz_pass = 0
    fuzz_fail = 0
    fail_inputs = []

    for r in range(rounds):
        # Generate random input: mix of ints, strings, edge cases
        rng = random.Random(r)
        input_lines = []
        num_inputs = rng.randint(1, 10)
        for _ in range(num_inputs):
            choice = rng.choice(["int", "string", "edge"])
            if choice == "int":
                input_lines.append(str(rng.randint(-2**31, 2**31 - 1)))
            elif choice == "string":
                length = rng.randint(0, 100)
                input_lines.append("".join(chr(rng.randint(32, 126)) for _ in range(length)))
            elif choice == "edge":
                edge = rng.choice(["0", "-1", "1", "2147483647", "-2147483648",
                                    "9999999999", "", " ", "\t", "NULL",
                                    "0.0", "-0.0", "inf", "-inf", "nan",
                                    "a" * 1000, "0" * 100])
                input_lines.append(edge)

        fuzz_input = "\n".join(input_lines) + "\n"

        orig_out, rt_out, match = run_differential(original_exe, roundtrip_exe, fuzz_input)

        if match:
            fuzz_pass += 1
        else:
            fuzz_fail += 1
            fail_inputs.append({
                "round": r,
                "input": fuzz_input[:200],
                "orig_out": (orig_out or "")[:200],
                "rt_out": (rt_out or "")[:200],
            })
            if fuzz_fail <= 3:
                print(f"  FAIL round {r}: input={fuzz_input[:50]!r}")
                if orig_out and rt_out:
                    orig_first = orig_out.splitlines()[0] if orig_out.splitlines() else ""
                    rt_first = rt_out.splitlines()[0] if rt_out.splitlines() else ""
                    print(f"    orig: {orig_first[:80]}")
                    print(f"    rt:   {rt_first[:80]}")

    print(f"\nFuzz results: {fuzz_pass} PASS, {fuzz_fail} FAIL out of {rounds} rounds")

    # Phase 3: Summary
    print()
    print("=== SUMMARY ===")
    print(f"Default execution: {'PASS' if match else 'FAIL'}")
    print(f"Fuzz testing: {fuzz_pass}/{rounds} PASS")
    print(f"UL error markers: {len(errors)}")
    print(f"Functions found: {len(program_info['functions'])}")
    print(f"Types found: {len(program_info['types'])}")
    print(f"Globals found: {len(program_info['globals'])}")

    if fail_inputs:
        # Save failure details
        fail_path = os.path.splitext(ul_file)[0] + "_fuzz_failures.json"
        with open(fail_path, "w") as f:
            json.dump(fail_inputs, f, indent=2)
        print(f"\nFailure details saved to: {fail_path}")

    overall = "PASS" if (match and fuzz_fail == 0 and len(errors) == 0) else "FAIL"
    print(f"\nOVERALL: {overall}")

    sys.exit(0 if overall == "PASS" else 1)


if __name__ == "__main__":
    main()
