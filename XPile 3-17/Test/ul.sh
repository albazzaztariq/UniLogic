#!/usr/bin/env bash
# ul.sh — UniLogic compiler wrapper for Unix
# Usage: ul run hello.ul, ul build, ul test, etc.
# Place this in your PATH or alongside Main.py

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Check same directory as this script
if [ -f "$SCRIPT_DIR/Main.py" ]; then
    MAIN_PY="$SCRIPT_DIR/Main.py"
elif [ -n "$UNILOGIC_HOME" ] && [ -f "$UNILOGIC_HOME/Main.py" ]; then
    MAIN_PY="$UNILOGIC_HOME/Main.py"
else
    echo "Error: Cannot find Main.py"
    echo "Set UNILOGIC_HOME or place this script alongside Main.py."
    exit 1
fi

exec python3 "$MAIN_PY" "$@"
