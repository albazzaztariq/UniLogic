"""
hasher.py — deterministic SHA-256 hashing utilities.

Every hash in MemResistant is canonical: we always hash UTF-8 encoded text
after normalising line endings to LF.  This means the same source file
produces the same hash on Windows, macOS, and Linux.
"""

import hashlib


def hash_file(path: str) -> str:
    """Return the hex SHA-256 of a file's contents (LF-normalised)."""
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()
    return hash_string(content)


def hash_string(text: str) -> str:
    """Return the hex SHA-256 of a string (LF-normalised)."""
    normalised = text.replace('\r\n', '\n').replace('\r', '\n')
    return hashlib.sha256(normalised.encode('utf-8')).hexdigest()


def hash_dict(d: dict) -> str:
    """Deterministically hash a JSON-serialisable dict."""
    import json
    canonical = json.dumps(d, sort_keys=True, separators=(',', ':'))
    return hash_string(canonical)
