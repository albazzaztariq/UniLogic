#!/usr/bin/env python3
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    lines = f.readlines()

# Test each line incrementally to find the exact failure
test_cases = [229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241]
for n in test_cases:
    chunk = ''.join(lines[:n])
    try:
        p = c_parser.CParser()
        p.parse(chunk, filename='test')
        print(f"Lines 1-{n}: PASS")
    except Exception as e:
        print(f"Lines 1-{n}: FAIL - {e}")
