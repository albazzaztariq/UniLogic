#!/usr/bin/env python3
"""Sync compiler source files into the VS Code extension bundle."""
import shutil, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEST = os.path.join(os.path.dirname(os.path.abspath(__file__)), "vscode-ul", "compiler")

FILES = [
    ("Lexer", "lexer.py"),
    ("Parser", "parser.py"),
    ("AST", "ast_nodes.py"),
    ("Semantic", "semcheck.py"),
    ("Codegen", "codegen_c.py"),
    ("Compiler", "normalize.py"),
]

os.makedirs(DEST, exist_ok=True)
for subdir, filename in FILES:
    src = os.path.join(ROOT, subdir, filename)
    dst = os.path.join(DEST, filename)
    shutil.copy2(src, dst)
    print(f"  {subdir}/{filename} -> compiler/{filename}")
print(f"Synced {len(FILES)} files to {DEST}")
