#!/usr/bin/env python3
"""Filter gcc -E preprocessed output to only keep lines from specified source files."""
import re
import sys
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))

src_files = {'cJSON.c', 'cJSON.h'}

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_full_preproc.txt'), 'r') as f:
    lines = f.readlines()

in_src = False
out = []
for line in lines:
    m = re.match(r'^# \d+ "(.+?)"', line)
    if m:
        fpath = m.group(1)
        fname = fpath.replace('\\', '/').split('/')[-1]
        in_src = fname in src_files
        continue
    if in_src:
        stripped = line.strip()
        if stripped.startswith('#pragma'):
            continue
        out.append(line)

result = ''.join(out)
with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_filtered.txt'), 'w') as f:
    f.write(result)
print(f"Filtered lines: {len(out)}")
print("First 3000 chars:")
print(result[:3000])
