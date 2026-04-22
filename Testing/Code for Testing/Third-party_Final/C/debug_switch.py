#!/usr/bin/env python3
"""Debug the switch/case AST structure for the fall-through switch in print_string_ptr."""
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser, c_ast

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    content = f.read()

p = c_parser.CParser()
ast = p.parse(content, filename='test')

# Find print_string_ptr
for node in ast.ext:
    if isinstance(node, c_ast.FuncDef) and node.decl.name == 'print_string_ptr':
        print(f"Found print_string_ptr")
        # Walk body to find switch
        def find_switches(compound, depth=0):
            if not compound or not compound.block_items:
                return
            for item in compound.block_items:
                if isinstance(item, c_ast.Switch):
                    print(f"\nFound Switch at depth {depth}")
                    if isinstance(item.stmt, c_ast.Compound):
                        for ci in item.stmt.block_items:
                            if isinstance(ci, c_ast.Case):
                                expr_str = str(ci.expr.value if hasattr(ci.expr, 'value') else ci.expr)
                                stmts_types = [type(s).__name__ for s in (ci.stmts or [])]
                                real = [s for s in (ci.stmts or []) if not isinstance(s, c_ast.Break)]
                                print(f"  Case {expr_str}: stmts={stmts_types}, real={[type(s).__name__ for s in real]}, stmts_is_None={ci.stmts is None}")
                            elif isinstance(ci, c_ast.Default):
                                print(f"  Default: stmts={[type(s).__name__ for s in (ci.stmts or [])]}")
                elif isinstance(item, c_ast.For):
                    if isinstance(item.stmt, c_ast.Compound):
                        find_switches(item.stmt, depth+1)
                elif isinstance(item, c_ast.While):
                    if isinstance(item.stmt, c_ast.Compound):
                        find_switches(item.stmt, depth+1)
                elif isinstance(item, c_ast.If):
                    if isinstance(item.iftrue, c_ast.Compound):
                        find_switches(item.iftrue, depth+1)
        find_switches(node.body)
        break
