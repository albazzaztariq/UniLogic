#!/usr/bin/env python3
"""Debug what the preprocessed output looks like before pycparser sees it."""
import sys
import os

_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))

from c2ul import preprocess_c_source

source_path = os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cJSON', 'cJSON.c')

with open(source_path, 'r') as f:
    source = f.read()

result = preprocess_c_source(source, source_path=source_path)
print(f"Preprocessed length: {len(result)} chars, {result.count(chr(10))} lines")
print("=== FIRST 4000 CHARS ===")
print(result[:4000])
print("=== LAST 1000 CHARS ===")
print(result[-1000:])

# Save full output for inspection
with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'w') as f:
    f.write(result)
print("\nSaved full output to cjson_preproc_final.txt")
