#!/usr/bin/env python3
"""Translate just print_string_ptr through the full C2UL flow."""
import sys
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))
from pycparser import c_parser, c_ast
from c2ul import C2UL

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    preprocessed = f.read()

p = c_parser.CParser()
ast = p.parse(preprocessed, filename='test')

translator = C2UL()
translator._emitted_types = set()

# Visit ONLY print_string_ptr
for node in ast.ext:
    if isinstance(node, c_ast.FuncDef) and node.decl.name == 'print_string_ptr':
        translator.visit_FuncDef(node)
        print("Generated UL for print_string_ptr:")
        for i, line in enumerate(translator.lines, 1):
            print(f"{i:4d}: {line}")
        break
