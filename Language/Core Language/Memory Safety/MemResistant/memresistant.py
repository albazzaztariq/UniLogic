"""
MemResistant — Static Analysis Attestation Tool

Runs a three-layer analysis pipeline on a C source file, builds a
cryptographically fingerprinted attestation, and optionally publishes
it to IPFS and/or GitHub.

Usage:
  python memresistant.py <file.c>                   Run analysis, print report
  python memresistant.py <file.c> --publish         Run + publish to IPFS+GitHub
  python memresistant.py <file.c> --ipfs            Run + publish to IPFS only
  python memresistant.py <file.c> --github          Run + publish to GitHub only
  python memresistant.py <file.c> --tsan            Also run ThreadSanitizer pass
  python memresistant.py <file.c> --save <out.json> Save attestation JSON locally
  python memresistant.py --verify <att.json>        Verify a saved attestation fingerprint

Layers
------
  1. CBMC  — static bounded model checking (no binary needed)
  2. ASan  — AddressSanitizer + UBSan (compile + run instrumented binary)
  3. TSan  — ThreadSanitizer (optional, --tsan flag, separate from ASan)

Trust model
-----------
The attestation includes:
  - SHA-256 of the source file (the anchor — changing one byte changes this)
  - Full stdout/stderr of each tool (deterministic — anyone can re-run)
  - SHA-256 of each tool's output
  - A fingerprint = SHA-256 of all of the above

Anyone can verify by re-running the listed tool versions on a file
matching the source hash and comparing outputs.  Nothing is hidden.
"""

import sys
import os

BASE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(BASE))

from MemResistant.core.attestation import build_attestation, save_attestation, \
                                          load_attestation, verify_fingerprint
from MemResistant.core.report       import generate_report, generate_summary
from MemResistant.tools.cbmc_runner import run_cbmc
from MemResistant.tools.asan_runner import run_asan
from MemResistant.tools.tsan_runner import run_tsan
from MemResistant.publish.ipfs_client   import publish_to_ipfs
from MemResistant.publish.github_client import publish_to_github


def run_analysis(c_file: str, include_tsan: bool = False) -> object:
    print(f"\nMemResistant  →  {c_file}\n")

    results = []

    print("  [1/3] CBMC static analysis...")
    results.append(run_cbmc(c_file))
    _status(results[-1])

    print("  [2/3] ASan + UBSan (compile + run)...")
    results.append(run_asan(c_file))
    _status(results[-1])

    if include_tsan:
        print("  [3/3] ThreadSanitizer (compile + run)...")
        results.append(run_tsan(c_file))
        _status(results[-1])
    else:
        print("  [3/3] TSan skipped (pass --tsan to enable)")

    print()
    att = build_attestation(c_file, results)
    print(generate_report(att))
    return att


def _status(result):
    icon = "  OK  " if result.passed else " FAIL "
    print(f"        [{icon}] {result.tool}")
    if not result.passed:
        for line in (result.stderr or '').strip().splitlines()[:5]:
            print(f"               {line}")


def main():
    args = sys.argv[1:]
    if not args or args[0] in ('-h', '--help'):
        print(__doc__)
        sys.exit(0)

    flags = {a for a in args if a.startswith('--')}

    # -- Verify mode --
    if '--verify' in flags:
        idx = args.index('--verify')
        if idx + 1 >= len(args):
            print("Error: --verify requires a path to an attestation JSON file.")
            sys.exit(1)
        att = load_attestation(args[idx + 1])
        ok = verify_fingerprint(att)
        print(f"Fingerprint check: {'PASS — attestation is intact' if ok else 'FAIL — attestation has been tampered with'}")
        sys.exit(0 if ok else 1)

    # -- Analysis mode --
    positional = [a for a in args if not a.startswith('--')]
    if not positional:
        print("Error: provide a .c file to analyse.")
        sys.exit(1)

    c_file = positional[0]
    if not os.path.exists(c_file):
        print(f"Error: file not found: {c_file!r}")
        sys.exit(1)

    att = run_analysis(c_file, include_tsan='--tsan' in flags)

    # -- Save locally --
    if '--save' in flags:
        idx = args.index('--save')
        save_path = args[idx + 1] if idx + 1 < len(args) and not args[idx + 1].startswith('--') \
                    else os.path.splitext(c_file)[0] + '.attestation.json'
        save_attestation(att, save_path)
        print(f"\nSaved attestation: {save_path}")

    # -- Publish --
    ipfs_cid = None
    if '--publish' in flags or '--ipfs' in flags:
        print("\nPublishing to IPFS...")
        ipfs_cid = publish_to_ipfs(att)

    if '--publish' in flags or '--github' in flags:
        print("Publishing to GitHub...")
        publish_to_github(att, ipfs_cid=ipfs_cid)

    sys.exit(0 if att.overall_pass else 1)


if __name__ == '__main__':
    main()
