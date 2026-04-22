# Celery Real-World Validation Results
**Date:** 2026-03-21
**Compiler:** UniLogic XPile
**Python version:** 3.12
**Source:** github.com/celery/celery (shallow clone, 2026-03-21)

---

## Overview

Three simplified celery modules tested through the full pipeline:
`Python source -> py2ul -> UL -> Python codegen -> run`

Stresses: async pattern (via `visit_AsyncFunctionDef = visit_FunctionDef`),
decorator-as-registration, signal dispatch, *args/**kwargs forwarding,
module-level imperative init, deep exception hierarchy.

| Test | Source Pattern | Python Round-Trip | Warnings |
|------|---------------|-------------------|----------|
| celery/exceptions.py | Deep inheritance, exception hierarchy, *args/__init__ | PASS | 0 |
| celery/states.py | Module-level constants, dict from loop, frozenset->list, precedence dispatch | PASS | 0 |
| celery/utils/functional.py | *args forwarding, decorator-as-registration, signal dispatch, callable objects | PASS | 0 |

---

## Test 1: celery/exceptions.py

**Source:** test_celery_exceptions.py (simplified from celery/exceptions.py)
**Patterns:** Deep exception class hierarchy (7 levels), `*args`/`**kwargs` in `__init__`,
class-level field defaults (`message = None`, `when = None`), `isinstance`/`issubclass`,
`try/except` with typed handlers, `repr()` overrides.

### Python Round-Trip: PASS

```
CeleryError is Exception: True
TaskPredicate is CeleryError: True
Ignore is TaskPredicate: True
Retry is TaskPredicate: True
TaskError is CeleryError: True
ChordError is TaskError: True
BackendGetMetaError is BackendError: True
Reject reason: bad message
Reject requeue: True
Reject repr: reject requeue=True: bad message
Retry message: try again
Retry humanize: in 5s
Retry str: try again
Retry no-msg str: Retry in 10s
MaxRetries message: max exceeded
BackendGetMetaError task_id: task-abc
BackendStoreError state: FAILURE
BackendStoreError task_id: task-xyz
Caught ChordError as TaskError: chord failed
Caught BackendGetMetaError as BackendError: t-001
Caught Ignore as TaskPredicate: ok
Caught AlwaysEagerIgnored as CeleryWarning: ok
isinstance Ignore/TaskPredicate: True
isinstance Ignore/CeleryError: True
isinstance NotRegistered/TaskError: True
NotRegistered repr: Task never registered
```

**Adaptations:**
- Removed external imports (kombu, billiard, click).
- Removed multiple inheritance (e.g. `QueueNotFound(KeyError, TaskError)` -> single parent).
- Simplified `**kwargs` forwarding to explicit positional args.

---

## Test 2: celery/states.py

**Source:** test_celery_states.py (simplified from celery/states.py)
**Patterns:** Module-level string constants, `frozenset` -> list, `dict(zip(...))` ->
explicit for loop init, `PRECEDENCE_LOOKUP` built at module init time, `state` class
with all four comparison operators overriding precedence rules, `try/except KeyError`.

### Python Round-Trip: PASS

```
PENDING: PENDING
SUCCESS: SUCCESS
FAILURE: FAILURE
precedence(SUCCESS): 0
precedence(FAILURE): 1
precedence(PENDING): 8
precedence(unknown): 2
NONE_PRECEDENCE: 2
SUCCESS in READY_STATES: True
PENDING in READY_STATES: False
PENDING in UNREADY_STATES: True
FAILURE in EXCEPTION_STATES: True
FAILURE in EXCEPTION_STATES: True
FAILURE in PROPAGATE_STATES: True
REVOKED in PROPAGATE_STATES: True
STARTED in PROPAGATE_STATES: False
state(PENDING) < state(SUCCESS): True
state(SUCCESS) > state(PENDING): True
state(FAILURE) > state(SUCCESS): False
state(SUCCESS) >= state(SUCCESS): True
state(SUCCESS) <= state(SUCCESS): True
state(PROGRESS) > state(STARTED): True
state(PROGRESS) > state(SUCCESS): False
PRECEDENCE length: 9
PRECEDENCE[0]: SUCCESS
PRECEDENCE[8]: PENDING
ALL_STATES length: 7
str(state(FAILURE)): FAILURE
repr(state(RETRY)): state(RETRY)
```

**Adaptations:**
- `frozenset({...})` -> `list` (UL has no frozenset type).
- `dict(zip(PRECEDENCE, range(...)))` replaced with explicit `for _i, _s in enumerate(PRECEDENCE)` loop.
- `None` sentinel in PRECEDENCE list replaced with `""` (UL arrays cannot hold `None`).

---

## Test 3: celery/utils/functional.py

**Source:** test_celery_functional.py (simplified from celery/utils/functional.py)
**Patterns:** `*args`/`**kwargs` forwarding, `noop(*args, **kwargs)`, `pass1(arg, *args)`,
`DummyContext` context manager, `firstmethod` dispatch pattern (callable object replaces
closure), `Registry` with nested `decorator` function (returned from method),
`Signal` send/connect/disconnect with `*args` dispatch.

### Python Round-Trip: PASS

```
noop: ok
pass1(42): 42
pass1(42, 1, 2): 42
maybe(to_str, 99): 99
maybe(to_str, None): None
first(None, none_items): found
first(is_positive, int_items): 5
first(is_positive, []): None
firstmethod get_val: celery
firstmethod with on_call: CELERY
padlist(['a','b','c'], 3): ['a', 'b', 'c']
padlist(['a','b'], 4, 'X'): ['a', 'b', 'X', 'X']
padlist(['a','b','c','d'], 2): ['a', 'b']
uniq([1,2,1,3,2]): [1, 2, 3]
uniq([]): []
uniq(['a','b','a']): ['a', 'b']
chunks([0..9], 3): [[0, 1, 2], [3, 4, 5], [6, 7, 8], [9]]
chunks([1,2,3], 2): [[1, 2], [3]]
chunks([], 3): []
DummyContext entered: True
DummyContext exited: ok
Registry names: ['add', 'greet']
dispatch add(3,4): 7
dispatch greet: hello celery
dispatch unknown: None
Signal send results count: 2
Signal result[0]: handled
Signal result[1]: also handled
Signal received args: ('result_value', 'worker1')
After disconnect count: 1
```

**Adaptations:**
- `firstmethod` returns a `Dispatcher` object instead of a closure (closures over
  parameters cannot be represented in UL's global-promotion closure model).
- Decorator syntax `@reg.register("add")` split to two-step call (UL decorators
  are warned/stripped; explicit `dec = reg.register("name"); dec(func)` works).
- Removed all external imports (kombu, vine, celery.utils.log).
- `str()` builtin replaced with `cast_str()` wrapper (str not UL-round-trippable).

---

## Bugs Fixed (this session, bugs 42-48)

| # | Location | Bug | Fix |
|---|----------|-----|-----|
| 42 | py2ul | Class-level bare assign (`message = ""`) AND `__init__` self.X assign both emitted → duplicate field | Track `class_assign_fields` set; exclude from `init_fields` emission |
| 43 | py2ul | Module-level `for` loop emitted at file scope → UL parser error | Redirect module-scope for/while loops to `__module_init__` via `_redirect_to_init` |
| 44 | py2ul | Module-level non-literal list with variable refs dropped (only comment emitted) | Detect `is_any_list`: emit `array T var` + populate in `__module_init__` |
| 45 | py2ul | Module-level dict assignment fell through `return` without emitting `map var = map_new()` | `is_dict` branch does not `return`; falls through to dict-literal handler |
| 46 | codegen_python | Module-level `array T var` with no init emitted as `var = 0` (int default) instead of `var = []` | Add `elif decl.type_.is_array and decl.init is None: emit f"{name} = []"` |
| 47 | codegen_python | `__module_init__()` generated but never called; module vars never initialized | Detect `__module_init__` in program.decls; call it before `main()` in `if __name__` block |
| 48 | codegen_python | `global` declaration includes function's own parameter names → Python SyntaxError | Subtract `param_names` from `used` before emitting `global` statement |
| 49 | codegen_python | Nested functions inside methods (stored in `nested_functions`) never emitted | Emit `method.nested_functions` before method body in `gen_object` |
| 50 | codegen_python | Nested `FunctionDecl` nodes in function bodies (via `gen_stmt`) not handled → silently dropped | Add `FunctionDecl` case to `gen_stmt` calling `gen_function(node)` |

---

## Known Limitations (not bugs — fundamental UL constraints)

1. **Closure-over-parameter**: `firstmethod` returns a closure capturing the outer
   parameter `method_name`. UL's global-promotion closure model cannot represent this.
   Workaround: use a callable `Dispatcher` object with explicit fields.

2. **Decorator syntax** `@reg.register("add")`: UL has no decorator syntax. py2ul emits
   a warning and skips. Workaround: two-step explicit call `dec = f(); dec(func)`.

3. **Chained call syntax** `f()(arg)`: parser uses `"__expr__"` as function name for
   non-Identifier call bases, producing broken Python output. Workaround: assign intermediate.

4. **async/await**: `visit_AsyncFunctionDef = visit_FunctionDef` — async functions are
   treated as sync (correct for round-trip; UL parallel syntax differs).

5. **Multiple inheritance**: only first base used; second ignored with warning.

6. **frozenset**: no UL equivalent — mapped to `array` (no deduplication at compile time).

---

## Files Modified

- `Tools/py2ul.py` — bugs 42, 43, 44, 45
- `Codegen/codegen_python.py` — bugs 46, 47, 48, 49, 50
