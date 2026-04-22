#!/usr/bin/env python3
"""Inspect the AST nodes in the first part of cJSON preprocessed file."""
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser, c_ast

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    content = f.read()

p = c_parser.CParser()
ast = p.parse(content, filename='test')

# Show first 30 top-level declarations
print("Top-level AST node types:")
for i, node in enumerate(ast.ext[:40]):
    node_type = type(node).__name__
    name = getattr(node, 'name', '')

    # Get more info for Decl nodes
    if isinstance(node, c_ast.Decl):
        decl_type = type(node.type).__name__
        quals = getattr(node, 'quals', [])
        storage = getattr(node, 'storage', [])

        # Check if it's a typedef
        is_typedef = 'typedef' in storage

        # Get inner type info
        inner = node.type
        if isinstance(inner, c_ast.TypeDecl):
            inner_type = type(inner.type).__name__
            if isinstance(inner.type, c_ast.Struct):
                struct_name = inner.type.name
                has_decls = inner.type.decls is not None
                print(f"  {i:2d}. Decl '{name}': {decl_type} -> {inner_type}(struct={struct_name}, has_body={has_decls}) typedef={is_typedef}")
            else:
                print(f"  {i:2d}. Decl '{name}': {decl_type} -> {inner_type} typedef={is_typedef}")
        elif isinstance(inner, c_ast.FuncDecl):
            print(f"  {i:2d}. Decl '{name}': FuncDecl typedef={is_typedef}")
        else:
            print(f"  {i:2d}. Decl '{name}': {decl_type} typedef={is_typedef}")
    else:
        print(f"  {i:2d}. {node_type} '{name}'")
