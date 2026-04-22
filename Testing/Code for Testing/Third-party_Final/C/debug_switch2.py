#!/usr/bin/env python3
"""Check what _visit_switch produces for the exact pycparser AST."""
import sys
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))
from pycparser import c_parser, c_ast
from c2ul import C2UL, preprocess_c_source

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    preprocessed = f.read()

p = c_parser.CParser()
ast = p.parse(preprocessed, filename='test')

translator = C2UL()

# Find print_string_ptr
for node in ast.ext:
    if isinstance(node, c_ast.FuncDef) and node.decl.name == 'print_string_ptr':
        print("Found print_string_ptr")
        # Find the switch and translate it
        def find_switch(compound):
            if not compound or not compound.block_items:
                return None
            for item in compound.block_items:
                if isinstance(item, c_ast.Switch):
                    return item
                elif isinstance(item, c_ast.For):
                    if isinstance(item.stmt, c_ast.Compound):
                        result = find_switch(item.stmt)
                        if result:
                            return result
                elif isinstance(item, c_ast.While):
                    if isinstance(item.stmt, c_ast.Compound):
                        result = find_switch(item.stmt)
                        if result:
                            return result
            return None

        sw = find_switch(node.body)
        if sw:
            print("Found switch, translating...")
            translator._visit_switch(sw)
            for line in translator.lines:
                print(line)
        break
