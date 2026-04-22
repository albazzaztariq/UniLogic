#!/usr/bin/env python3
"""Binary search for exact parse error location."""
import sys
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))
from pycparser import c_parser

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    content = f.read()

lines = content.split('\n')
total = len(lines)
print(f"Total lines: {total}")

# Binary search for the failing line
lo, hi = 0, total

def try_parse(n):
    chunk = '\n'.join(lines[:n])
    try:
        p = c_parser.CParser()
        p.parse(chunk, filename='test')
        return True
    except:
        return False

# Find first failing boundary
print("Binary searching for error boundary...")
lo, hi = 0, total
while lo < hi - 10:
    mid = (lo + hi) // 2
    if try_parse(mid):
        lo = mid
    else:
        hi = mid

print(f"Error occurs around line {hi}")
print(f"Lines {max(0, hi-20)} to {hi}:")
for i, l in enumerate(lines[max(0, hi-20):hi+5], start=max(0, hi-20)+1):
    print(f"  {i:4d}: {l}")
