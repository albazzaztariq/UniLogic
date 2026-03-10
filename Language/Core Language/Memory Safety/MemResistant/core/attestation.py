"""
attestation.py — build, save, and load MemResistant attestation objects.

An attestation is a JSON document that records:
  - the SHA-256 of the source file being verified
  - which tools ran, which versions, and whether they passed
  - the full stdout/stderr of each tool (so anyone can re-verify)
  - a hash of all of the above (the attestation fingerprint)

The fingerprint is the "seal".  If someone tampers with the JSON the
fingerprint will no longer match its contents.  Since CBMC output is
deterministic for a given input+version, anyone can independently
re-run the tools and compare fingerprints.

Trust model
-----------
MemResistant makes no cryptographic claims about *who* ran the tools.
It claims: "a file with this SHA-256 was run through these tool versions
and produced these outputs."  Anyone who doubts can reproduce it.
MemProof adds the blockchain timestamp + ZK layer on top.
"""

import json
import time
from dataclasses import dataclass, field, asdict
from typing import Optional
from .hasher import hash_file, hash_dict


@dataclass
class ToolResult:
    tool:       str             # "cbmc" | "asan" | "ubsan" | "tsan"
    version:    str
    passed:     bool
    stdout:     str
    stderr:     str
    output_hash: str = ""       # SHA-256 of stdout+stderr (filled on build)


@dataclass
class Attestation:
    schema_version: str = "memresistant-1.0"
    source_path:    str = ""
    source_hash:    str = ""    # SHA-256 of the source file
    timestamp:      int = 0     # Unix epoch seconds
    tool_results:   list = field(default_factory=list)   # List[ToolResult]
    overall_pass:   bool = False
    fingerprint:    str = ""    # SHA-256 of everything above (filled last)


def build_attestation(source_path: str,
                      tool_results: list[ToolResult]) -> Attestation:
    """Construct a complete, fingerprinted Attestation from tool results."""
    att = Attestation(
        source_path  = source_path,
        source_hash  = hash_file(source_path),
        timestamp    = int(time.time()),
        overall_pass = all(r.passed for r in tool_results),
    )
    # Attach output hashes to each result
    from .hasher import hash_string
    for r in tool_results:
        r.output_hash = hash_string(r.stdout + r.stderr)
    att.tool_results = [asdict(r) for r in tool_results]

    # Fingerprint = hash of everything except the fingerprint field itself
    body = asdict(att)
    body.pop('fingerprint', None)
    att.fingerprint = hash_dict(body)
    return att


def save_attestation(att: Attestation, path: str) -> None:
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(asdict(att), f, indent=2)


def load_attestation(path: str) -> Attestation:
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    results = [ToolResult(**r) for r in data.pop('tool_results', [])]
    att = Attestation(**data)
    att.tool_results = [asdict(r) for r in results]
    return att


def verify_fingerprint(att: Attestation) -> bool:
    """Re-compute the fingerprint and check it matches the stored one."""
    body = asdict(att)
    stored = body.pop('fingerprint', '')
    return hash_dict(body) == stored
