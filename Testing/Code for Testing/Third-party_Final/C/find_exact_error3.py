#!/usr/bin/env python3
"""Find exact parse failure by checking error messages carefully."""
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    lines = f.readlines()

total = len(lines)

# Try lines 700-800 checking every single line
print("Checking lines 700-800 one at a time...")
prev_was_ok = True
for i in range(700, 800):
    chunk = ''.join(lines[:i+1])
    try:
        p = c_parser.CParser()
        p.parse(chunk, filename='test')
        if not prev_was_ok:
            print(f"  Line {i+1}: recovered (ok)")
        prev_was_ok = True
    except Exception as e:
        msg = str(e)
        if "end of input" in msg.lower():
            prev_was_ok = True
            continue
        if prev_was_ok:
            print(f"  Line {i+1}: FAIL - {msg}")
            print(f"  Content: {repr(lines[i])}")
        prev_was_ok = False
