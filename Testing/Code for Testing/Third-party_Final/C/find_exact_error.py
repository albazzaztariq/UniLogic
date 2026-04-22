#!/usr/bin/env python3
"""Find exact parse failure by binary search."""
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    lines = f.readlines()

total = len(lines)
print(f"Total lines: {total}")

def try_parse(n):
    chunk = ''.join(lines[:n])
    try:
        p = c_parser.CParser()
        p.parse(chunk, filename='test')
        return True
    except Exception as e:
        msg = str(e)
        # "At end of input" means the chunk is truncated mid-construct - not a real error
        if "end of input" in msg.lower() or "eof" in msg.lower():
            return True  # treat as OK for search purposes
        return False

# Binary search
lo, hi = 237, total
while lo < hi - 1:
    mid = (lo + hi) // 2
    ok = try_parse(mid)
    print(f"  try {mid}: {'ok' if ok else 'FAIL'}", flush=True)
    if ok:
        lo = mid
    else:
        hi = mid

print(f"\nFirst real failure at or around line {hi}")
print("Context:")
for i, l in enumerate(lines[max(0, hi-10):hi+5], start=max(0, hi-10)+1):
    print(f"  {i:4d}: {l}", end='')
