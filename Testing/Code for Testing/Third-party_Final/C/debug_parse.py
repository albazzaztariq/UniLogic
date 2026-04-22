#!/usr/bin/env python3
"""Try to parse the filtered preprocessed output and show where it fails."""
import sys
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))

from pycparser import c_parser

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    source = f.read()

parser = c_parser.CParser()
try:
    ast = parser.parse(source, filename='cJSON.c')
    print("Parse SUCCEEDED")
except Exception as e:
    print(f"Parse FAILED: {e}")
    # Show context around the error
    lines = source.split('\n')
    print(f"\nTotal lines: {len(lines)}")

    # Try to find the error location from the error message
    import re
    m = re.search(r':(\d+):', str(e))
    if m:
        lineno = int(m.group(1))
        start = max(0, lineno - 5)
        end = min(len(lines), lineno + 5)
        print(f"\nContext around line {lineno}:")
        for i, l in enumerate(lines[start:end], start=start+1):
            marker = '>>>' if i == lineno else '   '
            print(f"{marker} {i:4d}: {l}")
    else:
        # Find problematic sections by trying to parse chunks
        print("\nSearching for problematic construct...")
        chunk_size = 200
        for i in range(0, len(lines), chunk_size):
            chunk = '\n'.join(lines[:i+chunk_size])
            try:
                parser2 = c_parser.CParser()
                parser2.parse(chunk, filename='test')
            except Exception as e2:
                err_str = str(e2)
                if 'Invalid' in err_str or 'error' in err_str.lower():
                    print(f"\nFailure around line {i+chunk_size}: {e2}")
                    # Show 10 lines before the end of the chunk
                    chunk_lines = chunk.split('\n')
                    for j, l in enumerate(chunk_lines[-15:], start=i+chunk_size-15):
                        print(f"  {j:4d}: {l}")
                    break
