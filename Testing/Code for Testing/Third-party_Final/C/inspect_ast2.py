#!/usr/bin/env python3
"""Inspect relevant AST nodes in cJSON preprocessed file - skip standard headers."""
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
from pycparser import c_parser, c_ast

with open(os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C', 'cjson_preproc_final.txt'), 'r') as f:
    content = f.read()

p = c_parser.CParser()
ast = p.parse(content, filename='test')

# Show all nodes after the standard fake_decls (first 45)
print("Top-level AST node types (from index 40 on):")
for i, node in enumerate(ast.ext[40:], start=40):
    node_type = type(node).__name__
    name = getattr(node, 'name', '')

    if isinstance(node, c_ast.Typedef):
        inner = node.type
        if isinstance(inner, c_ast.TypeDecl) and isinstance(inner.type, c_ast.Struct):
            struct = inner.type
            has_body = struct.decls is not None
            fields = []
            if has_body and struct.decls:
                for d in struct.decls:
                    field_type = type(d.type).__name__
                    fields.append(f"{d.name}:{field_type}")
            print(f"  {i:3d}. Typedef '{name}' -> struct '{struct.name}' body={has_body} fields={fields[:4]}")
        else:
            print(f"  {i:3d}. Typedef '{name}'")
    elif isinstance(node, c_ast.Decl):
        decl_type = type(node.type).__name__
        storage = getattr(node, 'storage', [])
        is_typedef = 'typedef' in storage
        inner = node.type
        if isinstance(inner, c_ast.TypeDecl) and isinstance(inner.type, c_ast.Struct):
            struct = inner.type
            print(f"  {i:3d}. Decl-struct '{name}' body={struct.decls is not None}")
        elif isinstance(inner, c_ast.FuncDecl):
            print(f"  {i:3d}. Decl-func '{name}'")
        else:
            print(f"  {i:3d}. Decl '{name}': {decl_type}")
    elif isinstance(node, c_ast.FuncDef):
        print(f"  {i:3d}. FuncDef '{node.decl.name}'")
    else:
        print(f"  {i:3d}. {node_type} '{name}'")
