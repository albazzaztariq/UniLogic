#!/usr/bin/env python3
"""Find parse failure by function boundaries — more accurate than line search."""
import re
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    content = f.read()

# Split at top-level declarations/functions by finding lines that start with
# a non-whitespace character (function definitions, typedef, struct defs)
lines = content.split('\n')

# Find indices of "top-level" boundaries: lines starting at column 0 that
# look like the start of a function or declaration
boundaries = [0]
brace_depth = 0
in_string = False
for i, line in enumerate(lines):
    stripped = line.strip()
    if stripped.startswith('//') or stripped.startswith('*') or not stripped:
        continue
    for ch in line:
        if ch == '"' and not in_string:
            in_string = True
        elif ch == '"' and in_string:
            in_string = False
        elif not in_string:
            if ch == '{':
                brace_depth += 1
            elif ch == '}':
                brace_depth -= 1
    if brace_depth == 0 and line and line[0] not in ' \t\n\r' and not stripped.startswith('#'):
        boundaries.append(i)

print(f"Found {len(boundaries)} top-level boundaries")
print(f"Total lines: {len(lines)}")

# Binary search for the failing boundary
def try_parse_to(end_line):
    chunk = '\n'.join(lines[:end_line])
    try:
        p = c_parser.CParser()
        p.parse(chunk, filename='test')
        return True
    except Exception as e:
        msg = str(e)
        return False

# Find which boundary first fails
lo_idx, hi_idx = 0, len(boundaries) - 1
while lo_idx < hi_idx - 1:
    mid_idx = (lo_idx + hi_idx) // 2
    end_line = boundaries[mid_idx]
    ok = try_parse_to(end_line)
    print(f"  boundary {mid_idx}/{len(boundaries)} (line {end_line}): {'ok' if ok else 'FAIL'}")
    if ok:
        lo_idx = mid_idx
    else:
        hi_idx = mid_idx

# Fine-grained check around the failure
fail_boundary = boundaries[hi_idx]
print(f"\nFirst failure near boundary index {hi_idx} (line {fail_boundary})")
print(f"Content around that line:")
for i, l in enumerate(lines[max(0, fail_boundary-5):fail_boundary+20], start=max(0, fail_boundary-5)):
    print(f"  {i+1:4d}: {l}")

# Try individual lines in that range
print("\nLine-by-line near failure:")
start = max(0, boundaries[lo_idx])
for i in range(start, min(fail_boundary+50, len(lines))):
    chunk = '\n'.join(lines[:i+1])
    try:
        p = c_parser.CParser()
        p.parse(chunk, filename='test')
    except Exception as e:
        msg = str(e)
        if 'invalid expression' in msg.lower() or 'before:' in msg.lower():
            # Try with one more closing brace to complete any open block
            chunk2 = chunk + '\n}'
            try:
                p2 = c_parser.CParser()
                p2.parse(chunk2, filename='test')
                # With } it parses — so it was just truncated
            except Exception as e2:
                msg2 = str(e2)
                if 'invalid expression' in msg2.lower() or 'before:' in msg2.lower():
                    print(f"  Line {i+1}: REAL FAIL - {msg}")
                    print(f"  Content: {repr(lines[i])}")
                    if i > start + 5:
                        break
