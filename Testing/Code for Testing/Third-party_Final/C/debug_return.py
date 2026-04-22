#!/usr/bin/env python3
"""Debug the return statement in cJSON_SetNumberHelper from full parse."""
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser, c_ast

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    content = f.read()

p = c_parser.CParser()
ast = p.parse(content, filename='test')

# Find cJSON_SetNumberHelper
for node in ast.ext:
    if isinstance(node, c_ast.FuncDef) and node.decl.name == 'cJSON_SetNumberHelper':
        print(f"Found cJSON_SetNumberHelper")
        body = node.body
        # Find return statement
        def find_returns(block):
            if not block or not block.block_items:
                return
            for item in block.block_items:
                if isinstance(item, c_ast.Return):
                    print(f"Return expr type: {type(item.expr).__name__}")
                    item.expr.show()
                elif isinstance(item, c_ast.If):
                    find_returns(item.iftrue if isinstance(item.iftrue, c_ast.Compound) else None)
        find_returns(body)
        break
