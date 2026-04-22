"""
Test file: simplified celery/utils/functional.py.
Self-contained, no external deps.
Stresses: *args/**kwargs forwarding, decorator-as-registration pattern,
          closures, generator-style functions, dynamic dispatch,
          list comprehensions, context manager protocol.
Adaptations:
  - Removed all external imports (kombu, vine, celery.utils.log).
  - Removed LRUCache, lazy, promise, memoize (require external libs).
  - Kept: noop, first, firstmethod, chunks, padlist, uniq, maybe.
  - DummyContext kept as context manager stress test.
  - firstmethod closure pattern: inner function capturing outer arg.
  - decorator_registry pattern: function-as-value stored in dict, then dispatched.
"""


# ── Simple utilities ─────────────────────────────────────────────────────────

def cast_str(x):
    """Convert x to string — used in place of str() which can't round-trip via UL."""
    return str(x)


def noop(*args, **kwargs):
    """No operation. Accepts anything, returns nothing."""
    pass


def pass1(arg, *args, **kwargs):
    """Return the first positional argument."""
    return arg


def maybe(typ, val):
    """Call typ(val) if val is not None, else return val."""
    if val is not None:
        return typ(val)
    return val


# ── first ─────────────────────────────────────────────────────────────────

def first(predicate, it):
    """Return first element in it where predicate(elem) is true.

    If predicate is None, return first non-None element.
    """
    for v in it:
        if predicate is None:
            if v is not None:
                return v
        else:
            if predicate(v):
                return v
    return None


# ── firstmethod: decorator-as-registration / closure dispatch ────────────

class Dispatcher:
    """Callable dispatcher returned by firstmethod.

    Holds method_name and on_call explicitly (no closure) so UL can represent it
    as an object with fields rather than a closure.
    Stresses: callable objects, getattr-based dynamic dispatch, *args forwarding.
    """

    def __init__(self, method_name="", on_call=None):
        self.method_name = method_name
        self.on_call = on_call

    def __call__(self, obj_list, *args, **kwargs):
        for obj in obj_list:
            meth = getattr(obj, self.method_name, None)
            if meth is None:
                continue
            try:
                if self.on_call is not None:
                    reply = self.on_call(meth)
                else:
                    reply = meth()
            except AttributeError:
                continue
            if reply is not None:
                return reply
        return None


def firstmethod(method_name, on_call=None):
    """Return a Dispatcher for method_name.

    Stresses: returning a callable object, function-as-value stored in variable.
    """
    return Dispatcher(method_name, on_call)


# ── padlist ───────────────────────────────────────────────────────────────

def padlist(container, size, default=None):
    """Pad list with default elements up to size."""
    result = list(container)
    while len(result) < size:
        result.append(default)
    return result[:size]


# ── uniq ──────────────────────────────────────────────────────────────────

def uniq(it):
    """Return unique elements preserving order."""
    seen = []
    result = []
    for obj in it:
        if obj not in seen:
            seen.append(obj)
            result.append(obj)
    return result


# ── chunks ────────────────────────────────────────────────────────────────

def chunks(items, n):
    """Split a list into chunks of n elements."""
    result = []
    i = 0
    while i < len(items):
        chunk = []
        j = i
        while j < len(items) and j < i + n:
            chunk.append(items[j])
            j += 1
        result.append(chunk)
        i += n
    return result


# ── DummyContext (context manager stress) ─────────────────────────────────

class DummyContext:
    """A no-op context manager."""

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        return False


# ── firstmethod test helpers (module-scope so UL can declare them) ───────

class HasName:
    """Helper for firstmethod tests."""

    def __init__(self, val=""):
        self.val = val

    def get_val(self):
        if self.val:
            return self.val
        return None


class NoName:
    """Helper with no get_val method."""
    pass


# ── Decorator-as-registration pattern ────────────────────────────────────
# Mirrors celery's task registry: @app.task registers a function by storing
# it in a dict. Stresses: function as value, *args forwarding, dict dispatch.

class Registry:
    """Simple task-style registry using decorator pattern."""

    def __init__(self):
        self._tasks = {}

    def register(self, name):
        """Decorator: register function under name."""
        def decorator(func):
            self._tasks[name] = func
            return func
        return decorator

    def dispatch(self, name, *args, **kwargs):
        """Dispatch call to registered function by name."""
        func = self._tasks.get(name)
        if func is None:
            return None
        return func(*args, **kwargs)

    def names(self):
        return list(self._tasks.keys())


# ── Signal dispatch pattern ───────────────────────────────────────────────
# Mirrors celery's signal.send(*args, **kwargs) pattern.

class Signal:
    """Minimal signal: connect receivers, send with *args forwarding."""

    def __init__(self, name=""):
        self.name = name
        self._receivers = []

    def connect(self, receiver):
        self._receivers.append(receiver)

    def send(self, *args, **kwargs):
        results = []
        for receiver in self._receivers:
            result = receiver(*args, **kwargs)
            results.append(result)
        return results

    def disconnect(self, receiver):
        if receiver in self._receivers:
            self._receivers.remove(receiver)


def main():
    # noop: accepts anything
    noop()
    noop(1, 2, 3)
    noop(a=1, b=2)
    print("noop: ok")

    # pass1
    print("pass1(42):", pass1(42))
    print("pass1(42, 1, 2):", pass1(42, 1, 2))

    # maybe — use a simple wrapper function instead of builtin str (str not UL-roundtrippable)
    def to_str(x):
        return cast_str(x)
    print("maybe(to_str, 99):", maybe(to_str, 99))
    print("maybe(to_str, None):", maybe(to_str, None))

    # first with predicate (homogeneous int list to avoid type errors in comparison)
    int_items = [0, -1, 5, 10]
    none_items = [None, None, "found", "second"]
    print("first(None, none_items):", first(None, none_items))

    def is_positive(x):
        return x > 0
    print("first(is_positive, int_items):", first(is_positive, int_items))
    print("first(is_positive, []):", first(is_positive, []))

    # firstmethod — closure dispatch using module-scope HasName / NoName
    dispatcher = firstmethod("get_val")
    objs = [NoName(), HasName(""), HasName("celery"), HasName("kombu")]
    print("firstmethod get_val:", dispatcher(objs))

    # firstmethod with on_call
    def call_upper(meth):
        result = meth()
        if result:
            return result.upper()
        return None
    dispatcher2 = firstmethod("get_val", on_call=call_upper)
    print("firstmethod with on_call:", dispatcher2(objs))

    # padlist
    print("padlist(['a','b','c'], 3):", padlist(["a", "b", "c"], 3))
    print("padlist(['a','b'], 4, 'X'):", padlist(["a", "b"], 4, "X"))
    print("padlist(['a','b','c','d'], 2):", padlist(["a", "b", "c", "d"], 2))

    # uniq
    print("uniq([1,2,1,3,2]):", uniq([1, 2, 1, 3, 2]))
    print("uniq([]):", uniq([]))
    print("uniq(['a','b','a']):", uniq(["a", "b", "a"]))

    # chunks
    print("chunks([0..9], 3):", chunks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], 3))
    print("chunks([1,2,3], 2):", chunks([1, 2, 3], 2))
    print("chunks([], 3):", chunks([], 3))

    # DummyContext
    with DummyContext() as ctx:
        print("DummyContext entered:", ctx is not None)
    print("DummyContext exited: ok")

    # Registry (explicit registration — decorator equivalent for UL round-trip)
    # Decorators like @reg.register("add") cannot be represented in UL;
    # explicit reg.register("add")(func) calls are the UL-compatible form.
    reg = Registry()

    def add(x, y):
        return x + y

    def greet(name):
        return "hello " + name

    # Explicit two-step registration (UL does not support chained call syntax a()(b))
    dec_add = reg.register("add")
    dec_add(add)
    dec_greet = reg.register("greet")
    dec_greet(greet)

    print("Registry names:", reg.names())
    print("dispatch add(3,4):", reg.dispatch("add", 3, 4))
    print("dispatch greet:", reg.dispatch("greet", "celery"))
    print("dispatch unknown:", reg.dispatch("missing", 1))

    # Signal dispatch with *args forwarding (no kwargs — py2ul converts kwargs to positional)
    sig = Signal("task_success")

    received = []

    def on_task_success(*args):
        received.append(args)
        return "handled"

    def on_task_success2(*args):
        return "also handled"

    sig.connect(on_task_success)
    sig.connect(on_task_success2)

    results = sig.send("result_value", "worker1")
    print("Signal send results count:", len(results))
    print("Signal result[0]:", results[0])
    print("Signal result[1]:", results[1])
    print("Signal received args:", received[0])

    sig.disconnect(on_task_success)
    results2 = sig.send("another")
    print("After disconnect count:", len(results2))


main()
