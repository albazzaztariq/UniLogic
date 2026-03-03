"""
Pragma Transpiler — C target

Usage:
  python pragma.py <source.run>              Transpile to <source.c>
  python pragma.py <source.run> out.c        Transpile to a specific path
  python pragma.py <source.run> --run        Transpile and compile+run with gcc
  python pragma.py <source.run> --ast        Print the AST (debug)
  python pragma.py <source.run> --tokens     Print the token stream (debug)

Optimisation flags (require --run):
  --lto                Link-time optimisation (-flto).  Lets gcc optimise across
                       translation units at link time.  Free ~5-15% in most cases.
  --pgo-record         Instrument the binary to record branch/call profiles
                       (-fprofile-generate).  Run the resulting exe with
                       representative workloads, then recompile with --pgo-use.
  --pgo-use            Compile using a previously recorded profile
                       (-fprofile-use -fprofile-correction).  Typically gains
                       another 10-20% on top of -O2, especially branch prediction.

PGO workflow:
  1.  python pragma.py prog.run --run --pgo-record   # builds + runs instrumented exe
  2.  (run the exe with your real workload to generate *.gcda profile files)
  3.  python pragma.py prog.run --run --pgo-use       # builds optimised exe

Safety / analysis flags:
  --fullsafety         Three-layer safety pass (use during development/testing):
                         1. Emit runtime bounds checks on all array subscripts
                         2. Run CBMC static analysis on the generated C
                            (requires cbmc on PATH; skipped with a warning if absent)
                         3. Compile with ASan + UBSan
                            (-fsanitize=address,undefined -fno-omit-frame-pointer)
                       Cannot be combined with --tsan (ASan/TSan conflict) or --pgo-use.
  --tsan               Compile with ThreadSanitizer (-fsanitize=thread).
                       For threaded code.  Mutually exclusive with --fullsafety.

Note: --fullsafety and --tsan are dev/test flags.  Do NOT ship these binaries.
      --lto and --pgo-use are production flags.  Do not mix sanitizers with PGO.
"""

import sys
import os
import subprocess

from lexer import Lexer
from parser import parse, ParseError
from codegen import generate


def transpile(source: str, safe: bool = False) -> str:
    program = parse(source)
    return generate(program, safe=safe)


def main():
    args = sys.argv[1:]

    if not args:
        print(__doc__)
        sys.exit(1)

    src_path = args[0]
    if not os.path.exists(src_path):
        print(f"Error: file not found: {src_path!r}")
        sys.exit(1)

    with open(src_path, 'r', encoding='utf-8') as f:
        source = f.read()

    flags    = {a for a in args[1:] if a.startswith('--')}
    positional = [a for a in args[1:] if not a.startswith('--')]

    # ── Debug modes ──────────────────────────────────────────────────────────

    if '--tokens' in flags:
        tokens = Lexer(source).tokenize()
        for tok in tokens:
            print(f"  {tok.line:3}  {tok.type.name:<18} {tok.value!r}")
        return

    if '--ast' in flags:
        import pprint
        program = parse(source)
        pprint.pprint(program)
        return

    # ── Validate flag combinations ────────────────────────────────────────────

    if '--fullsafety' in flags and '--tsan' in flags:
        print("Error: --fullsafety (includes ASan) and --tsan cannot be combined.")
        sys.exit(1)

    if ('--fullsafety' in flags or '--tsan' in flags) and '--pgo-use' in flags:
        print("Error: sanitizers and --pgo-use cannot be combined.")
        sys.exit(1)

    # ── Transpile ────────────────────────────────────────────────────────────

    safe_mode = '--fullsafety' in flags

    try:
        c_src = transpile(source, safe=safe_mode)
    except ParseError as e:
        print(f"Parse error: {e}")
        sys.exit(1)

    out_path = positional[0] if positional else os.path.splitext(src_path)[0] + '.c'

    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(c_src)

    print(f"Transpiled:  {src_path}  ->  {out_path}")

    # ── CBMC static analysis (--fullsafety) ──────────────────────────────────

    if '--fullsafety' in flags:
        import shutil
        if shutil.which('cbmc'):
            print("Running CBMC static analysis...")
            cbmc_flags = [
                '--bounds-check',
                '--overflow-check',
                '--div-by-zero-check',
                '--pointer-check',
                '--memory-leak-check',
            ]
            r = subprocess.run(['cbmc', *cbmc_flags, out_path], text=True)
            if r.returncode != 0:
                print("CBMC: verification FAILED — fix the above before continuing.")
                sys.exit(r.returncode)
            print("CBMC: OK")
        else:
            print("Warning: cbmc not found on PATH — static analysis skipped.")
            print("         Install with: sudo apt install cbmc  (or brew install cbmc)")

    # ── Compile + run ────────────────────────────────────────────────────────

    if '--run' in flags:
        exe = os.path.splitext(out_path)[0]

        gcc_flags = ['-O2', '-Wall']

        if '--lto' in flags:
            gcc_flags.append('-flto')

        if '--pgo-record' in flags and '--pgo-use' in flags:
            print("Error: --pgo-record and --pgo-use are mutually exclusive.")
            sys.exit(1)

        if '--pgo-record' in flags:
            gcc_flags.append('-fprofile-generate')

        if '--pgo-use' in flags:
            gcc_flags += ['-fprofile-use', '-fprofile-correction']

        if '--fullsafety' in flags:
            gcc_flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']

        if '--tsan' in flags:
            gcc_flags.append('-fsanitize=thread')

        desc = ' '.join(gcc_flags)
        print(f"Compiling with gcc ({desc})...")
        r = subprocess.run(
            ['gcc', *gcc_flags, '-o', exe, out_path],
            text=True
        )
        if r.returncode != 0:
            sys.exit(r.returncode)
        print(f"Running {exe}...")
        subprocess.run([exe])


if __name__ == '__main__':
    main()
