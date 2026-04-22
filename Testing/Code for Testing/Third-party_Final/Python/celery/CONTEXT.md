# Celery Test Context
**Date:** 2026-03-21
**Session:** celery real-world stress testing

---

## What Was Tested

celery/celery is a distributed task queue library. Its core modules stress patterns
that are common in production async Python code and difficult for transpilers:

### celery/exceptions.py
A deep exception hierarchy with:
- 20+ exception classes across 6 inheritance levels
- Class-level field defaults (`message = None`, `when = None`) alongside `__init__` self.X assignments
- `*args/**kwargs` forwarding to `super().__init__()`
- `isinstance`/`issubclass` checks, `try/except` typed handlers

### celery/states.py
Module-level initialization patterns:
- `dict(zip(list, range(...)))` — dict built from a zip of a list + counter
- Module-level imperative code (for loop populating a dict at import time)
- `frozenset({VAR1, VAR2, ...})` — set literals with variable references
- Custom `state` class subclassing `str` with all 4 comparison operator overloads
- `try/except KeyError` for dict lookup with fallback

### celery/utils/functional.py
Functional patterns:
- `noop(*args, **kwargs)`, `pass1(arg, *args, **kwargs)` — variadics
- `firstmethod(method_name)` — returns a dispatcher (closure-over-parameter pattern)
- `DummyContext` — `__enter__`/`__exit__` context manager
- `Registry` with nested `decorator` function returned from a method
- `Signal.send(*args)` — signal dispatch with *args forwarding to receivers

---

## Source Files (not modified)

```
celery_src/celery/exceptions.py
celery_src/celery/states.py
celery_src/celery/utils/functional.py
```

## Test Files Created

```
test_celery_exceptions.py     — simplified exceptions module, self-contained
test_celery_states.py         — simplified states module, self-contained
test_celery_functional.py     — simplified functional module, self-contained
```

## Generated Files

```
test_celery_exceptions.ul     — py2ul output
test_celery_states.ul         — py2ul output
test_celery_functional.ul     — py2ul output
test_celery_exceptions_rt.py  — UL -> Python codegen output
test_celery_states_rt.py      — UL -> Python codegen output
test_celery_functional_rt.py  — UL -> Python codegen output
```

---

## Key Transpiler Findings

### Closure-over-parameter: fundamental limitation
`firstmethod(method_name)` returns a closure `_matcher` that captures `method_name`
from the outer function's parameter scope. UL promotes free variables to module-level
globals, but does NOT inject setter code in the enclosing function. This means the
global `method_name` stays `None` when `_matcher` is called. No fix was applied
(would require major rework); workaround is to use a callable object with explicit fields.

### Module-level imperative code: new feature added (bug 43)
`PRECEDENCE_LOOKUP = {}; for i, s in enumerate(PRECEDENCE): PRECEDENCE_LOOKUP[s] = i`
— a for loop at file scope breaks the UL parser (only declarations valid at file scope).
Fix: redirect module-scope for/while loops to `__module_init__` via a temporary buffer.

### Class-level field defaults: dedup bug fixed (bug 42)
`class Retry: message = ""; when = ""` declares class-level defaults AND `__init__`
assigns `self.message = message`. Both paths emit `string message` → duplicate field.
Fix: collect class-level bare assigns into `class_assign_fields` set and exclude from
`init_fields` emission.

### `__module_init__` not called: codegen bug fixed (bug 47)
py2ul generates `__module_init__()` for module-scope imperative code, but codegen_python
never called it. All module vars (PRECEDENCE_LOOKUP, READY_STATES, etc.) stayed at
their default empty values. Fix: detect `__module_init__` in decls; call before `main()`.

### Nested functions in method bodies: codegen bug fixed (bugs 49, 50)
The parser extracts nested `FunctionDecl` nodes from function bodies into
`decl.nested_functions` (separate from `body`). codegen_python's `gen_object` method
emitted method.body but not method.nested_functions — the `decorator` nested function
inside `Registry.register()` was silently dropped. Fix: emit `nested_functions` before
the method body, both in `gen_object` (for methods) and via `gen_stmt` (for function bodies).

### `global` statement conflicts with parameter names: Python SyntaxError (bug 48)
codegen_python emits `global x, y` for module vars used in a function. If `x` is also
a parameter name, Python raises `SyntaxError: name 'x' is parameter and global`.
Fix: subtract `param_names` from the set of used globals before emitting.

---

## Pipeline

```
Python source -> py2ul -> .ul -> Main.py -t python -> _rt.py -> python _rt.py
```

All three tests: Python source PASS -> UL compiles -> Python round-trip PASS.
