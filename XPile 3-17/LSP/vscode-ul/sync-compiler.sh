#!/bin/bash
# Syncs compiler source files from the main project into the bundled compiler/ directory.
# Run this before packaging the vsix whenever the compiler is updated.

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEST="$(dirname "$0")/compiler"

cp "$ROOT/Lexer/lexer.py" \
   "$ROOT/Parser/parser.py" \
   "$ROOT/AST/ast_nodes.py" \
   "$ROOT/Semantic/semcheck.py" \
   "$ROOT/Codegen/codegen_c.py" \
   "$ROOT/Compiler/normalize.py" \
   "$DEST/"

echo "Synced $(ls "$DEST"/*.py | wc -l) compiler files to $DEST"
