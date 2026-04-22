#!/usr/bin/env python3
"""Find exact parse failure by binary search - improved to handle incremental parsing."""
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
        return True, None
    except Exception as e:
        msg = str(e)
        if "end of input" in msg.lower() or "eof" in msg.lower():
            return True, None  # incomplete input - not a real error
        return False, msg

# Fine-grained search from the last known good point
lo, hi = 677, total
last_good = 677

# Print lines around boundaries
print("Scanning for error...")
step = 10
i = lo
while i < min(lo + 300, total):
    ok, err = try_parse(i)
    if not ok:
        print(f"  Line {i}: FAIL - {err}")
        # Find exact line in this range
        for j in range(max(last_good, i-step), i+1):
            ok2, err2 = try_parse(j)
            if not ok2:
                print(f"\nFirst failure at line {j}: {err2}")
                print("Context around failure:")
                for k, l in enumerate(lines[max(0, j-15):j+5], start=max(0, j-15)+1):
                    print(f"  {k:4d}: {l}", end='')
                break
        break
    else:
        last_good = i
        print(f"  Line {i}: ok")
    i += step
