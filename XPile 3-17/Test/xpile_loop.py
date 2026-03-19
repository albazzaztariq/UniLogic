#!/usr/bin/env python3
# xpile_loop.py — Two-instance Claude orchestration loop
# Opus = architect (plans, critiques, decides when done)
# Sonnet = implementer (writes code, edits files)
# Usage: python xpile_loop.py
# Set ANTHROPIC_API_KEY in your environment before running.

import os
import sys
import json
import re
import anthropic
from pathlib import Path

# ── Config ────────────────────────────────────────────────────────────────────

PROJECT_DIR  = Path(r"C:\Users\azt12\OneDrive\Documents\Computing\Pragma\CURRENT\XPile 3-17")
MAX_ITERS    = 3
ARCHITECT    = "claude-opus-4-6"
IMPLEMENTER  = "claude-sonnet-4-6"
LOG_FILE     = PROJECT_DIR / "loop_log.jsonl"

FOLDERS = {
    "Lexer":    PROJECT_DIR / "Lexer",
    "Parser":   PROJECT_DIR / "Parser",
    "AST":      PROJECT_DIR / "AST",
    "Semantic": PROJECT_DIR / "Semantic",
    "Codegen":  PROJECT_DIR / "Codegen",
}

# ── Anthropic client ──────────────────────────────────────────────────────────

client = anthropic.Anthropic(api_key=os.environ.get("ANTHROPIC_API_KEY"))

# ── File utilities ────────────────────────────────────────────────────────────

def read_project_files():
    """Read all .py files from the project folders and Main.py."""
    files = {}
    for folder_name, folder_path in FOLDERS.items():
        if folder_path.exists():
            for f in folder_path.glob("*.py"):
                key = f"{folder_name}/{f.name}"
                files[key] = f.read_text(encoding="utf-8")
    main = PROJECT_DIR / "Main.py"
    if main.exists():
        files["Main.py"] = main.read_text(encoding="utf-8")
    return files

def write_file(relative_path, content):
    """Write content to a file given a path like 'Codegen/codegen_c.py'."""
    parts = relative_path.replace("\\", "/").split("/")
    if len(parts) == 1:
        full_path = PROJECT_DIR / parts[0]
    else:
        folder = parts[0]
        filename = parts[1]
        folder_path = FOLDERS.get(folder, PROJECT_DIR / folder)
        folder_path.mkdir(exist_ok=True)
        full_path = folder_path / filename
    full_path.write_text(content, encoding="utf-8")
    print(f"  wrote: {relative_path}")

def extract_file_blocks(text):
    """
    Parse file blocks from model output.
    Expected format:
        FILE: path/to/file.py
        ```python
        ...code...
        ```
    Returns dict of {path: content}.
    """
    files = {}
    pattern = re.compile(
        r"FILE:\s*(\S+)\s*\n```(?:\w+)?\n(.*?)```",
        re.DOTALL
    )
    for m in pattern.finditer(text):
        path    = m.group(1).strip()
        content = m.group(2)
        files[path] = content
    return files

def format_files_for_prompt(files):
    """Format file dict into a readable block for prompts."""
    blocks = []
    for path, content in files.items():
        blocks.append(f"FILE: {path}\n```python\n{content}\n```")
    return "\n\n".join(blocks)

# ── Logging ───────────────────────────────────────────────────────────────────

def log(entry):
    with open(LOG_FILE, "a", encoding="utf-8") as f:
        f.write(json.dumps(entry) + "\n")

# ── Model calls ───────────────────────────────────────────────────────────────

def call(model, system, messages):
    response = client.messages.create(
        model=model,
        max_tokens=4096,
        system=system,
        messages=messages,
    )
    return response.content[0].text

# ── Architect system prompt ───────────────────────────────────────────────────

ARCHITECT_SYSTEM = """You are the architect of the XPile compiler — a UniLogic (.ul) to C/Python/JavaScript transpiler.
Your job is to:
1. Receive a task from the user.
2. Break it into a clear implementation plan.
3. Review code produced by the implementer and decide if it is correct and complete.
4. If not correct, give precise, specific feedback — point to exact lines or logic errors.
5. When the implementation is correct and complete, respond with exactly: DONE

Rules:
- Be direct and specific. No filler.
- When giving feedback, say exactly what is wrong and exactly what to do.
- Do not write code yourself. That is the implementer's job.
- When you respond DONE, the loop stops and changes are written to disk."""

# ── Implementer system prompt ─────────────────────────────────────────────────

IMPLEMENTER_SYSTEM = """You are the implementer of the XPile compiler — a UniLogic (.ul) to C/Python/JavaScript transpiler.
Your job is to:
1. Receive an implementation plan or feedback from the architect.
2. Write or modify the code to fulfill it exactly.
3. Output ONLY the files you changed, in this exact format:

FILE: Folder/filename.py
```python
...complete file content...
```

Rules:
- Always output the COMPLETE file content, not just the changed lines.
- Only output files that actually changed.
- No explanation before or after unless asked.
- The project structure is:
    Main.py          — compiler driver
    Lexer/lexer.py   — tokenizer
    Parser/parser.py — recursive descent parser
    AST/ast_nodes.py — AST node dataclasses
    Semantic/semcheck.py  — semantic checker
    Codegen/codegen_c.py  — C code generator"""

# ── Main loop ─────────────────────────────────────────────────────────────────

def run_task(task):
    print(f"\n{'='*60}")
    print(f"TASK: {task}")
    print(f"{'='*60}\n")

    files = read_project_files()
    file_context = format_files_for_prompt(files)

    # Architect: plan the task
    arch_messages = [
        {
            "role": "user",
            "content": f"Here is the current state of the XPile compiler:\n\n{file_context}\n\nTask: {task}\n\nProvide an implementation plan for the implementer."
        }
    ]

    print(f"[Architect] Planning...")
    arch_reply = call(ARCHITECT, ARCHITECT_SYSTEM, arch_messages)
    print(f"[Architect]\n{arch_reply}\n")
    log({"role": "architect", "iteration": 0, "content": arch_reply})

    # Implementer/Architect iteration loop
    impl_messages = [
        {
            "role": "user",
            "content": f"Here is the current state of the XPile compiler:\n\n{file_context}\n\nImplementation plan from architect:\n{arch_reply}\n\nImplement this now."
        }
    ]

    written_files = {}

    for iteration in range(1, MAX_ITERS + 1):
        print(f"[Implementer] Iteration {iteration}...")
        impl_reply = call(IMPLEMENTER, IMPLEMENTER_SYSTEM, impl_messages)
        print(f"[Implementer]\n{impl_reply}\n")
        log({"role": "implementer", "iteration": iteration, "content": impl_reply})

        # Extract and stage any file changes
        new_files = extract_file_blocks(impl_reply)
        if new_files:
            written_files.update(new_files)
            print(f"  [{len(new_files)} file(s) staged]")

        # Architect reviews
        arch_messages = [
            {
                "role": "user",
                "content": f"Original task: {task}\n\nImplementer output (iteration {iteration}):\n{impl_reply}\n\nReview this. If correct and complete, respond DONE. If not, give specific feedback."
            }
        ]
        print(f"[Architect] Reviewing iteration {iteration}...")
        arch_reply = call(ARCHITECT, ARCHITECT_SYSTEM, arch_messages)
        print(f"[Architect]\n{arch_reply}\n")
        log({"role": "architect", "iteration": iteration, "content": arch_reply})

        if "DONE" in arch_reply:
            print("[Architect] Approved. Writing files to disk...")
            for path, content in written_files.items():
                write_file(path, content)
            print(f"\nDone in {iteration} iteration(s).")
            return

        # Feed architect feedback back to implementer
        impl_messages.append({"role": "assistant", "content": impl_reply})
        impl_messages.append({"role": "user", "content": f"Architect feedback:\n{arch_reply}\n\nRevise and output the corrected files."})

    # Hit max iterations without approval
    print(f"\n[Loop] Hit max iterations ({MAX_ITERS}) without architect approval.")
    if written_files:
        answer = input("Write staged files anyway? (y/n): ").strip().lower()
        if answer == "y":
            for path, content in written_files.items():
                write_file(path, content)
    print("Stopping.")


def main():
    print("XPile Claude Loop")
    print("Architect: Opus  |  Implementer: Sonnet")
    print("Type your task. Empty line to quit.\n")

    while True:
        try:
            task = input("task> ").strip()
        except (KeyboardInterrupt, EOFError):
            break
        if not task:
            break
        run_task(task)


if __name__ == "__main__":
    main()
