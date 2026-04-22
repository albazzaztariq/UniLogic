#!/usr/bin/env python3
"""
UniLogic Compiler Orchestration MCP Server
Manages task assignment across multiple Claude Code instances.
Workers poll for tasks, report results, escalate blockers to human.

Usage:
    python ul_orchestrator.py

Exposes HTTP endpoints on localhost:7771
Workers call these endpoints to get tasks and report results.
"""

import json
import sys
import time
import uuid
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from datetime import datetime
from pathlib import Path

# ─── State ───────────────────────────────────────────────────────────────────

STATE_FILE = Path("ul_orchestrator_state.json")

INITIAL_TASKS = [
    {
        "id": "T001",
        "title": "with statement without as binding",
        "description": (
            "Parser currently requires 'with expr as name' — make 'as name' optional. "
            "When no binding: Python emits 'with expr:', C emits enter/exit without var, "
            "JS emits try/finally without binding. "
            "Test: with suppress(ImportError) block compiles and runs on all targets."
        ),
        "files": ["Parser/parser.py", "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_with.ul still passes. 'with expr' without 'as' compiles on all 3 targets.",
        "needs_human": False,
        "priority": 1,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T002",
        "title": "nocache volatile variable keyword",
        "description": (
            "Add 'nocache int x = 0' keyword. "
            "C: emit as 'volatile int x = 0'. Python/JS: regular var with comment. "
            "Semcheck: nocache + constant on same var is an error. "
            "Write test_nocache.ul."
        ),
        "files": ["Lexer/lexer.py", "Parser/parser.py", "AST/ast_nodes.py", "Semantic/semcheck.py",
                  "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_nocache.ul compiles and runs on all targets. C output contains 'volatile'.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T003",
        "title": "Bit fields (uint.bits=N syntax)",
        "description": (
            "Add 'uint.bits=N fieldname' inside type declarations. "
            "Parser: recognize uint.bits=N as type annotation on field. "
            "AST: add bit_width field to field declarations. "
            "C: emit as C bit fields 'unsigned int active : 1'. "
            "Python/JS: regular int fields with bit width comment. "
            "Semcheck: verify bit widths 1-64. "
            "Write test_bitfields.ul — declare type with bit fields, set/read values."
        ),
        "files": ["Lexer/lexer.py", "Parser/parser.py", "AST/ast_nodes.py", "Semantic/semcheck.py",
                  "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_bitfields.ul compiles on all targets. C output has ': N' bit field syntax.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T004",
        "title": "@packed annotation for struct packing",
        "description": (
            "@packed on a type disables struct padding. "
            "C: emit __attribute__((packed)) on struct. "
            "Python/JS: comment only. "
            "Semcheck: warn if @packed used without fixed-width fields. "
            "Write test_packed.ul."
        ),
        "files": ["Parser/parser.py", "Semantic/semcheck.py", "Codegen/codegen_c.py"],
        "acceptance": "test_packed.ul compiles. C output has __attribute__((packed)).",
        "needs_human": False,
        "priority": 3,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T005",
        "title": "#function macro syntax",
        "description": (
            "Add '#function name(params) = expr' at file scope. "
            "Restriction: expression macros only — no conditionals (use @embedcall for those). "
            "Parser: recognize #function at file scope. "
            "AST: MacroDecl(name, params, body_expr). "
            "All codegens: substitute expression at call sites with parameter substitution. "
            "No function declaration emitted — pure substitution. "
            "Write test_macro.ul."
        ),
        "files": ["Lexer/lexer.py", "Parser/parser.py", "AST/ast_nodes.py", "Semantic/semcheck.py",
                  "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_macro.ul compiles. No function declaration in C output — expression inlined at call site.",
        "needs_human": False,
        "priority": 3,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T006",
        "title": "@symbol annotation wired in codegen",
        "description": (
            "Currently @symbol is parsed but has zero effect. Wire it up. "
            "Semcheck: when @symbol(op) present, register function as operator for that type pair. "
            "When 'v1 + v2' where both are Vector, look up @symbol(+) for Vector, rewrite to vector_add(v1, v2). "
            "C: emit function normally, rewrite operator at call sites. "
            "Python: emit __add__, __sub__, __mul__ etc as methods on class. "
            "JS: static method + rewrite operators at call sites. "
            "Write test_symbol.ul — Vector with @symbol(+) and @symbol(*), verify v1 + v2 works."
        ),
        "files": ["Semantic/semcheck.py", "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_symbol.ul compiles and runs correctly on all targets. v1 + v2 produces correct result.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T007",
        "title": "Pipe operator |> verified end-to-end",
        "description": (
            "The |> operator is in spec — verify it actually works in all codegens. "
            "'5 |> double |> add_one |> square' == square(add_one(double(5))). "
            "Parser: |> is left-associative binary operator rewriting to func(expr). "
            "Write test_pipe.ul with chain of 4 functions. "
            "Run on all targets and verify output is correct."
        ),
        "files": ["Parser/parser.py", "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_pipe.ul compiles and produces correct output on all targets.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T008",
        "title": "cast() full coverage",
        "description": (
            "Improve cast() to handle all type combinations: "
            "int->float, float->int (truncate), any->string, string->int, string->float, "
            "bool->int, int->bool. "
            "C: (float)x, (int)x, sprintf, atoi, atof, (x!=0). "
            "Python: float(x), int(x), str(x), bool(x). "
            "JS: Number(x), Math.trunc(x), String(x), parseInt(x), parseFloat(x), Boolean(x). "
            "Write test_cast.ul — test every combination. Verify correct output all targets."
        ),
        "files": ["Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_cast.ul all combinations produce correct output on all targets.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T009",
        "title": "@onlyone singleton fully implemented",
        "description": (
            "Currently @onlyone just stores annotation. Implement fully. "
            "C: emit static instance var + Config_instance() accessor function. "
            "   Direct construction error in semcheck: '@onlyone object must be accessed via Type.instance()'. "
            "Python: class with _instance=None + @classmethod instance(). "
            "JS: module-level var + static instance() method. "
            "Write test_onlyone.ul — call .instance() twice, verify same object, verify field persistence."
        ),
        "files": ["Semantic/semcheck.py", "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_onlyone.ul compiles and runs. Two .instance() calls return same object. Direct construction is a semcheck error.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T010",
        "title": "@allownull nullable fields fully implemented",
        "description": (
            "Currently @allownull just stores annotation. Implement fully. "
            "C: @allownull string fields emit as char* initialized to NULL. "
            "   Non-nullable fields emit as values. "
            "Semcheck: @allownull field accessed without null check -> warning. "
            "          Non-@allownull field assigned empty/none -> error. "
            "Python: @allownull fields initialize to None. Non-nullable to zero value. "
            "Write test_allownull.ul — type with nullable and non-nullable fields, "
            "assign empty to nullable, verify null check works."
        ),
        "files": ["Semantic/semcheck.py", "Codegen/codegen_c.py", "Codegen/codegen_python.py", "Codegen/codegen_js.py"],
        "acceptance": "test_allownull.ul compiles. C output has char* for nullable string fields. Assigning empty to non-nullable field is a semcheck error.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T011",
        "title": "Transpiler: dict .items(), chained comparisons",
        "description": (
            "Two transpiler fixes in Tools/py2ul.py: "
            "Fix 1: for key, val in d.items() -> map_keys loop with map_get. "
            "Fix 2: chained comparisons '0 < x < 10' -> '0 < x and x < 10'. "
            "  Detect ast.Compare with multiple comparators, expand to and-chained binary comparisons. "
            "After fixes: rerun sklearn/utils/validation.py and report updated line/warning/compile status."
        ),
        "files": ["Tools/py2ul.py"],
        "acceptance": "sklearn file gets further than line 141. dict.items() translates correctly. Chained comparisons translate correctly.",
        "needs_human": False,
        "priority": 1,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T012",
        "title": "Real Python file gauntlet — find and fix blockers",
        "description": (
            "Run the transpiler against 5 real Python files from GitHub that were NOT written for UL. "
            "Find files that are: pure Python (no C extensions called directly), "
            "have type annotations, do real computation. "
            "Suggested sources: "
            "- A file from https://github.com/psf/requests "
            "- A file from https://github.com/pallets/flask "
            "- A file from https://github.com/python/cpython/Lib/ "
            "- A file from https://github.com/numpy/numpy (pure Python parts only) "
            "- One file of your choice. "
            "For each file: run transpiler, attempt to compile output, report results. "
            "Fix the top 3 most common failure patterns found. "
            "Retest after fixes. Report before/after table."
        ),
        "files": ["Tools/py2ul.py"],
        "acceptance": "At least 2 of 5 real files compile and run correctly with correct output.",
        "needs_human": False,
        "priority": 3,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T013",
        "title": "Real C library integration test",
        "description": (
            "Write a UL program that calls real C stdlib functions via FFI — not UL stdlib wrappers. "
            "The program should: "
            "1. Use qsort() directly via FFI to sort an array of 1000 random ints "
            "2. Use time() and difftime() to measure elapsed time "
            "3. Use sprintf() to format a result string "
            "4. Use fopen/fwrite/fclose to write results to a file "
            "5. Read the file back and verify contents "
            "This proves UL can call arbitrary C library functions, not just its own wrappers. "
            "Compile to C and verify the program runs correctly end-to-end."
        ),
        "files": ["Programs/test_real_c_ffi.ul", "Stdlib/"],
        "acceptance": "Program compiles to C, runs without errors, file is created with correct content, output matches expected.",
        "needs_human": False,
        "priority": 2,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T014",
        "title": "Full regression suite — all targets including bytecode",
        "description": (
            "Run the complete regression suite across ALL targets: "
            "C, Python, JS, Interpreter, Bytecode VM. "
            "Current known state: 117/117 on C/Python/JS. Bytecode has known FFI limitation. "
            "Goal: verify nothing regressed after all the above tasks complete. "
            "Fix any regressions found. "
            "Report full table with all targets."
        ),
        "files": ["Programs/"],
        "acceptance": "All tests pass on C, Python, JS, Interpreter. Bytecode passes everything except FFI (by design).",
        "needs_human": False,
        "priority": 4,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
    {
        "id": "T015",
        "title": "HUMAN DECISION: async concurrency model for C target",
        "description": (
            "Currently @async functions in C target compile synchronously with a warning. "
            "Three options: "
            "1. Keep as-is — C developers use threaded concurrency, async is Python/JS only "
            "2. Implement async via libuv or libevent for C target "
            "3. Implement async via setjmp/longjmp cooperative scheduling "
            "This is a design decision that affects language guarantees. Human input required."
        ),
        "files": [],
        "acceptance": "Human makes decision. Implement accordingly.",
        "needs_human": True,
        "priority": 3,
        "status": "pending",
        "assigned_to": None,
        "started_at": None,
        "finished_at": None,
        "result": None,
        "test_count": None,
    },
]

lock = threading.Lock()


def load_state():
    if STATE_FILE.exists():
        with open(STATE_FILE) as f:
            return json.load(f)
    return {
        "tasks": INITIAL_TASKS,
        "test_count": 117,
        "log": [],
        "started_at": datetime.now().isoformat(),
    }


def save_state(state):
    with open(STATE_FILE, "w") as f:
        json.dump(state, f, indent=2)


state = load_state()


# ─── Helpers ─────────────────────────────────────────────────────────────────

def log_event(msg):
    entry = {"time": datetime.now().isoformat(), "msg": msg}
    state["log"].append(entry)
    print(f"[{entry['time']}] {msg}")
    save_state(state)


def get_next_task(worker_id):
    with lock:
        # Prioritize by priority number (lower = higher priority)
        pending = [t for t in state["tasks"]
                   if t["status"] == "pending" and not t["needs_human"]]
        if not pending:
            return None
        pending.sort(key=lambda t: t["priority"])
        task = pending[0]
        task["status"] = "in_progress"
        task["assigned_to"] = worker_id
        task["started_at"] = datetime.now().isoformat()
        log_event(f"Task {task['id']} '{task['title']}' assigned to worker '{worker_id}'")
        save_state(state)
        return task


def mark_task_done(task_id, worker_id, result, test_count):
    with lock:
        task = next((t for t in state["tasks"] if t["id"] == task_id), None)
        if not task:
            return False, "task not found"
        task["status"] = "done"
        task["finished_at"] = datetime.now().isoformat()
        task["result"] = result
        task["test_count"] = test_count
        if test_count:
            state["test_count"] = test_count
        log_event(f"Task {task_id} DONE by '{worker_id}'. Tests: {test_count}. Result: {result[:100]}")
        save_state(state)
        return True, "ok"


def mark_task_blocked(task_id, worker_id, reason):
    with lock:
        task = next((t for t in state["tasks"] if t["id"] == task_id), None)
        if not task:
            return False, "task not found"
        task["status"] = "blocked"
        task["result"] = reason
        log_event(f"Task {task_id} BLOCKED by '{worker_id}': {reason}")
        save_state(state)
        return True, "ok"


def get_status():
    with lock:
        tasks = state["tasks"]
        return {
            "test_count": state["test_count"],
            "started_at": state["started_at"],
            "summary": {
                "total": len(tasks),
                "pending": len([t for t in tasks if t["status"] == "pending"]),
                "in_progress": len([t for t in tasks if t["status"] == "in_progress"]),
                "done": len([t for t in tasks if t["status"] == "done"]),
                "blocked": len([t for t in tasks if t["status"] == "blocked"]),
                "needs_human": len([t for t in tasks if t["needs_human"] and t["status"] == "pending"]),
            },
            "tasks": tasks,
            "recent_log": state["log"][-20:],
        }


def add_task(title, description, files, acceptance, priority=3, needs_human=False):
    with lock:
        task_id = f"T{len(state['tasks'])+1:03d}"
        task = {
            "id": task_id,
            "title": title,
            "description": description,
            "files": files,
            "acceptance": acceptance,
            "needs_human": needs_human,
            "priority": priority,
            "status": "pending",
            "assigned_to": None,
            "started_at": None,
            "finished_at": None,
            "result": None,
            "test_count": None,
        }
        state["tasks"].append(task)
        log_event(f"Task {task_id} '{title}' added")
        save_state(state)
        return task


def resolve_blocked(task_id, decision):
    with lock:
        task = next((t for t in state["tasks"] if t["id"] == task_id), None)
        if not task:
            return False, "task not found"
        task["status"] = "pending"
        task["needs_human"] = False
        task["description"] += f"\n\nHUMAN DECISION: {decision}"
        log_event(f"Task {task_id} unblocked with decision: {decision}")
        save_state(state)
        return True, "ok"


# ─── HTTP Handler ─────────────────────────────────────────────────────────────

class Handler(BaseHTTPRequestHandler):

    def log_message(self, format, *args):
        pass  # suppress default access log

    def send_json(self, code, data):
        body = json.dumps(data, indent=2).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", len(body))
        self.end_headers()
        self.wfile.write(body)

    def read_body(self):
        length = int(self.headers.get("Content-Length", 0))
        if length:
            return json.loads(self.rfile.read(length))
        return {}

    def do_GET(self):
        if self.path == "/status":
            self.send_json(200, get_status())

        elif self.path.startswith("/next_task"):
            worker_id = self.path.split("?worker=")[-1] if "?" in self.path else "unknown"
            task = get_next_task(worker_id)
            if task:
                self.send_json(200, task)
            else:
                self.send_json(200, {"status": "no_tasks", "message": "All tasks complete or in progress"})

        elif self.path == "/rules":
            self.send_json(200, RULES)

        elif self.path == "/blockers":
            blocked = [t for t in state["tasks"] if t["status"] == "blocked" or t["needs_human"]]
            self.send_json(200, {"blockers": blocked})

        else:
            self.send_json(404, {"error": "unknown endpoint"})

    def do_POST(self):
        body = self.read_body()

        if self.path == "/done":
            ok, msg = mark_task_done(
                body.get("task_id"),
                body.get("worker_id", "unknown"),
                body.get("result", ""),
                body.get("test_count"),
            )
            self.send_json(200 if ok else 400, {"ok": ok, "message": msg})

        elif self.path == "/blocked":
            ok, msg = mark_task_blocked(
                body.get("task_id"),
                body.get("worker_id", "unknown"),
                body.get("reason", ""),
            )
            self.send_json(200 if ok else 400, {"ok": ok, "message": msg})

        elif self.path == "/add_task":
            task = add_task(
                body.get("title", "Untitled"),
                body.get("description", ""),
                body.get("files", []),
                body.get("acceptance", ""),
                body.get("priority", 3),
                body.get("needs_human", False),
            )
            self.send_json(200, task)

        elif self.path == "/resolve":
            ok, msg = resolve_blocked(
                body.get("task_id"),
                body.get("decision", ""),
            )
            self.send_json(200 if ok else 400, {"ok": ok, "message": msg})

        elif self.path == "/update_tests":
            with lock:
                state["test_count"] = body.get("count", state["test_count"])
                save_state(state)
            self.send_json(200, {"ok": True, "test_count": state["test_count"]})

        else:
            self.send_json(404, {"error": "unknown endpoint"})


# ─── Rules document served to workers ────────────────────────────────────────

RULES = {
    "project": "UniLogic (UL) compiler — XPile",
    "repo": "https://github.com/albazzaztariq/UniLogic",
    "compiler_dir": "XPile 3-17",
    "python": sys.executable,
    "language_rules": [
        "NO braces anywhere in UL syntax. Ever. Not in examples, not in generated code comments.",
        "NO string interpolation. Use + concatenation only.",
        "NO ternary operator in UL.",
        "NO lambdas/anonymous functions in UL.",
        "end keyword closes all blocks: end function, end if, end while, end try, etc.",
        "main() is required as entry point.",
        "type = zero-overhead C struct. object = full OOP with vtable.",
        "xor is primary bitwise XOR keyword (diff and delta are aliases).",
        "goto/portal for unconditional jumps.",
        "lock replaces Mutex.",
        "userinput() replaces args().",
        "vault() for environment variable access.",
        "list.drop() replaces list.pop().",
    ],
    "workflow_rules": [
        "Read ALL relevant files completely before modifying anything.",
        "Run full regression suite after EVERY task. Zero regressions allowed.",
        "If tests regress, fix before moving on. Do not report done with regressions.",
        "If a task requires a fundamental language design decision, call /blocked with reason.",
        "Do not invent syntax. If unsure whether something is valid UL, call /blocked.",
        "Test on ALL targets: C, Python, JS, Interpreter. Bytecode where applicable.",
        "After completing a task, call /done with test_count and result summary.",
        "Then immediately call /next_task to get the next assignment.",
        "Do not stop working until /next_task returns no_tasks.",
    ],
    "test_command": "python Main.py test",
    "compile_command": "python Main.py <file.ul> -t <c|python|js> -o <output>",
    "run_command": "python Main.py <file.ul> --run",
    "current_test_count": "117/117 on C, Python, JS. Bytecode 8/9 (FFI by design).",
}


# ─── Main ─────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    PORT = 7771
    server = HTTPServer(("localhost", PORT), Handler)
    print(f"""
╔══════════════════════════════════════════════════════════╗
║         UniLogic Orchestration Server v1.0               ║
╠══════════════════════════════════════════════════════════╣
║  Running on http://localhost:{PORT}                        ║
║                                                          ║
║  Worker endpoints:                                       ║
║    GET  /rules              — get language + workflow    ║
║    GET  /next_task?worker=X — get next task              ║
║    POST /done               — mark task complete         ║
║    POST /blocked            — escalate to human          ║
║                                                          ║
║  Human endpoints:                                        ║
║    GET  /status             — full project state         ║
║    GET  /blockers           — items needing decision     ║
║    POST /add_task           — add new task               ║
║    POST /resolve            — unblock a task             ║
║    POST /update_tests       — update test count          ║
║                                                          ║
║  Tasks loaded: {len(state['tasks'])}                                   ║
║  Current test count: {state['test_count']}/117                      ║
╚══════════════════════════════════════════════════════════╝
""")
    log_event("Orchestration server started")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")
        save_state(state)
