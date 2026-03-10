"""
MemProof — Formal Verification + Blockchain Attestation Tool

Runs a full formal verification pipeline on a C source file, then creates
a cryptographically anchored attestation stored on IPFS with an immutable
timestamp recorded on-chain (Ethereum or any EVM chain).

Pipeline
--------
  1. CBMC       Static bounded model checking
  2. ASan+UBSan Runtime instrumented build + execution
  3. LLM → WP  LLM generates ACSL annotations; Frama-C/WP proves them
               (iterative feedback loop, up to 5 rounds)
  4. IPFS       Full proof bundle uploaded → CID returned
  5. Chain      CID + source hash recorded on MemProofRegistry smart contract

Usage:
  python memproof.py <file.c>             Full pipeline, print results
  python memproof.py <file.c> --attest    Full pipeline + IPFS + on-chain
  python memproof.py <file.c> --wp-only   Skip CBMC/ASan, only run WP
  python memproof.py --query <hash>       Query chain for existing attestation

Environment variables required for --attest:
  MEMPROOF_LLM_URL        (default: http://localhost:11434 — Ollama)
  MEMPROOF_LLM_MODEL      (default: deepseek-coder:6.7b)
  PINATA_JWT              IPFS pinning service token
  ETH_RPC_URL             EVM RPC endpoint
  ETH_PRIVATE_KEY         Wallet private key (hex)
  MEMPROOF_CONTRACT       Deployed MemProofRegistry address

What makes this different from MemResistant?
--------------------------------------------
MemResistant: "We ran the best static tools and they found nothing."
MemProof:     "We formally proved it — here is the mathematical proof,
               here is its IPFS address, here is its blockchain timestamp.
               You can verify all three independently without trusting us."
"""

import sys
import os
import json
import time
import tempfile
import dataclasses

BASE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(BASE))

from MemProof.tools.framac_runner    import run_framac
from MemProof.llm.feedback_loop      import annotate_and_verify
from MemProof.chain.ipfs_client      import publish_to_ipfs
from MemProof.chain.eth_client       import submit_attestation, query_attestation

# Re-use MemResistant's CBMC and ASan runners (they work on plain C)
from MemResistant.tools.cbmc_runner  import run_cbmc
from MemResistant.tools.asan_runner  import run_asan
from MemResistant.core.hasher        import hash_file, hash_dict
from MemResistant.core.attestation   import ToolResult


def _read_file(path: str) -> str:
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        return f.read()


def run_pipeline(c_file: str,
                 skip_cbmc: bool = False,
                 skip_asan: bool = False) -> dict:
    """
    Run the full MemProof pipeline.  Returns a proof bundle dict.
    """
    source = _read_file(c_file)
    source_hash = hash_file(c_file)

    print(f"\nMemProof  →  {c_file}")
    print(f"Source hash: {source_hash}\n")

    bundle: dict = {
        'schema':      'memproof-1.0',
        'source_hash': source_hash,
        'source_path': c_file,
        'timestamp':   int(time.time()),
        'cbmc':        None,
        'asan':        None,
        'wp':          None,
        'overall':     False,
    }

    # ── Layer 1: CBMC ────────────────────────────────────────────────────────
    if not skip_cbmc:
        print("  [1/3] CBMC static analysis...")
        cbmc = run_cbmc(c_file)
        bundle['cbmc'] = dataclasses.asdict(cbmc)
        print(f"        {'OK' if cbmc.passed else 'FAIL'}")
    else:
        print("  [1/3] CBMC skipped")

    # ── Layer 2: ASan + UBSan ────────────────────────────────────────────────
    if not skip_asan:
        print("  [2/3] ASan + UBSan...")
        asan = run_asan(c_file)
        bundle['asan'] = dataclasses.asdict(asan)
        print(f"        {'OK' if asan.passed else 'FAIL'}")
    else:
        print("  [2/3] ASan skipped")

    # ── Layer 3: LLM → Frama-C/WP ───────────────────────────────────────────
    print("  [3/3] LLM annotation + Frama-C/WP formal proof...")
    annotated, wp_result = annotate_and_verify(
        function_source = source,
        c_file_context  = source,
    )
    if wp_result:
        bundle['wp'] = {
            'passed':       wp_result.passed,
            'total_goals':  wp_result.total_goals,
            'valid_goals':  wp_result.valid_goals,
            'failed_count': wp_result.failed_count,
            'version':      wp_result.version,
            'annotated_source': annotated or '',
        }
        print(f"        {'OK' if wp_result.passed else 'FAIL'} "
              f"({wp_result.valid_goals}/{wp_result.total_goals} goals proved)")

    # ── Overall ──────────────────────────────────────────────────────────────
    cbmc_ok  = bundle['cbmc']['passed']  if bundle['cbmc'] else True
    asan_ok  = bundle['asan']['passed']  if bundle['asan'] else True
    wp_ok    = bundle['wp']['passed']    if bundle['wp']   else False
    bundle['overall'] = cbmc_ok and asan_ok and wp_ok

    status = "MEMPROOF PASS" if bundle['overall'] else "MEMPROOF FAIL"
    print(f"\n  {status}\n")
    return bundle


def main():
    args  = sys.argv[1:]
    flags = {a for a in args if a.startswith('--')}

    if not args or '-h' in args or '--help' in flags:
        print(__doc__)
        sys.exit(0)

    # ── Query mode ───────────────────────────────────────────────────────────
    if '--query' in flags:
        idx = args.index('--query')
        source_hash = args[idx + 1] if idx + 1 < len(args) else None
        if not source_hash:
            print("Error: --query requires a source hash (64-char hex)")
            sys.exit(1)
        result = query_attestation(source_hash)
        if result:
            print(json.dumps(result, indent=2))
        else:
            print(f"No on-chain attestation found for {source_hash}")
        sys.exit(0)

    # ── Analysis mode ────────────────────────────────────────────────────────
    positional = [a for a in args if not a.startswith('--')]
    if not positional:
        print("Error: provide a .c file.")
        sys.exit(1)

    c_file = positional[0]
    if not os.path.exists(c_file):
        print(f"Error: file not found: {c_file!r}")
        sys.exit(1)

    bundle = run_pipeline(
        c_file,
        skip_cbmc = '--wp-only' in flags,
        skip_asan = '--wp-only' in flags,
    )

    # ── Publish + attest ─────────────────────────────────────────────────────
    if '--attest' in flags:
        print("Publishing to IPFS...")
        cid = publish_to_ipfs(bundle)

        if cid:
            print("Recording on chain...")
            tx = submit_attestation(
                source_hash = bundle['source_hash'],
                ipfs_cid    = cid,
                passed      = bundle['overall'],
            )
            if tx:
                bundle['tx_hash'] = tx

        # Save bundle locally too
        out = os.path.splitext(c_file)[0] + '.memproof.json'
        with open(out, 'w', encoding='utf-8') as f:
            json.dump(bundle, f, indent=2)
        print(f"Proof bundle saved: {out}")

    sys.exit(0 if bundle['overall'] else 1)


if __name__ == '__main__':
    main()
