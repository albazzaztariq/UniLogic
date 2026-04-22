# @dr python_compat = true


def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

# python_compat type aliases (UL keywords remapped by py2ul)
_py_list = list
_py_map = dict
_py_type = type
_py_int = int
_py_float = float
_py_string = str
_py_bool = bool
_py_object = object

class Dispatcher:
    def __init__(self, method_name='', on_call=None):
        self.method_name = method_name
        self.on_call = on_call
    def __call__(self, obj_list, *_va_args):
        for obj in obj_list:
            meth = getattr(obj, self.method_name, None)
            if (meth == None):
                continue
            try:
                if (self.on_call != None):
                    reply = self.on_call(meth)
                else:
                    reply = meth()
            except Exception as __ex:
                __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
                __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
                if __ex_type == "AttributeError":
                    continue
                else:
                    raise
            if (reply != None):
                return reply
        return None

class DummyContext:
    def __init__(self):
        pass
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_val, exc_tb):
        return False

class HasName:
    def __init__(self, val=''):
        self.val = val
    def get_val(self):
        if self.val:
            return self.val
        return None

class NoName: pass  # stub for imported type

class Registry:
    def __init__(self):
        _dict0 = {}
        self._tasks = _dict0
    def register(self, name):
        def decorator(func):
            self._tasks[name] = func
            return func
        
        return decorator
    def dispatch(self, name, *_va_args):
        func = self._tasks.get(name)
        if (func == None):
            return None
        return func(*_va_args)
    def names(self):
        return list(self._tasks.keys())

class Signal:
    def __init__(self, name=''):
        self.name = name
        self._receivers = []
    def connect(self, receiver):
        self._receivers.append(receiver)
    def send(self, *_va_args):
        results = []
        for receiver in self._receivers:
            result = receiver(*_va_args)
            results.append(result)
        return results
    def disconnect(self, receiver):
        if (receiver in self._receivers):
            self._receivers.remove(receiver)

cast_str = None
received = None

def main__to_str(x):
    return cast_str(x)

def main__is_positive(x):
    return (x > 0)

def main__call_upper(meth):
    result = meth()
    if result:
        return result.upper()
    return None

def main__add(x, y):
    return (x + y)

def main__greet(name):
    return ('hello ' + name)

def main__on_task_success(*_va_args):
    global received
    received.append(_va_args)
    return 'handled'

def main__on_task_success2(*_va_args):
    return 'also handled'

def cast_str(x):
    return str(x)

def noop(*_va_args):
    0

def pass1(arg, *_va_args):
    return arg

def maybe(typ, val):
    if (val != None):
        return typ(val)
    return val

def first(predicate, it):
    for v in it:
        if (predicate == None):
            if (v != None):
                return v
        else:
            if predicate(v):
                return v
    return None

def firstmethod(method_name, on_call=None):
    return Dispatcher(method_name, on_call)

def padlist(container, _py_size, _py_default=None):
    result = list(container)
    while (len(result) < _py_size):
        result.append(_py_default)
    return result[0:_py_size]

def uniq(it):
    seen = []
    result = []
    for obj in it:
        if (not (obj in seen)):
            seen.append(obj)
            result.append(obj)
    return result

def chunks(items, n):
    result = []
    i = 0
    while (i < len(items)):
        chunk = []
        j = i
        while ((j < len(items)) and (j < (i + n))):
            chunk.append(items[j])
            j += 1
        result.append(chunk)
        i += n
    return result

def main():
    global received
    noop()
    noop(1, 2, 3)
    noop(1, 2)
    print('noop: ok')
    print(((str('pass1(42):') + ' ') + str(pass1(42))))
    print(((str('pass1(42, 1, 2):') + ' ') + str(pass1(42, 1, 2))))
    print(((str('maybe(to_str, 99):') + ' ') + str(maybe(main__to_str, 99))))
    print(((str('maybe(to_str, None):') + ' ') + str(maybe(main__to_str, None))))
    int_items = [0, (-1), 5, 10]
    none_items = [None, None, 'found', 'second']
    print(((str('first(None, none_items):') + ' ') + str(first(None, none_items))))
    print(((str('first(is_positive, int_items):') + ' ') + str(first(main__is_positive, int_items))))
    print(((str('first(is_positive, []):') + ' ') + str(first(main__is_positive, []))))
    dispatcher = firstmethod('get_val', None)
    objs = [NoName(), HasName(''), HasName('celery'), HasName('kombu')]
    print(((str('firstmethod get_val:') + ' ') + str(dispatcher(objs))))
    dispatcher2 = firstmethod('get_val', main__call_upper)
    print(((str('firstmethod with on_call:') + ' ') + str(dispatcher2(objs))))
    print(((str("padlist(['a','b','c'], 3):") + ' ') + str(padlist(['a', 'b', 'c'], 3, None))))
    print(((str("padlist(['a','b'], 4, 'X'):") + ' ') + str(padlist(['a', 'b'], 4, 'X'))))
    print(((str("padlist(['a','b','c','d'], 2):") + ' ') + str(padlist(['a', 'b', 'c', 'd'], 2, None))))
    print(((str('uniq([1,2,1,3,2]):') + ' ') + str(uniq([1, 2, 1, 3, 2]))))
    print(((str('uniq([]):') + ' ') + str(uniq([]))))
    print(((str("uniq(['a','b','a']):") + ' ') + str(uniq(['a', 'b', 'a']))))
    print(((str('chunks([0..9], 3):') + ' ') + str(chunks([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], 3))))
    print(((str('chunks([1,2,3], 2):') + ' ') + str(chunks([1, 2, 3], 2))))
    print(((str('chunks([], 3):') + ' ') + str(chunks([], 3))))
    ctx = DummyContext()
    try:
        print(((str('DummyContext entered:') + ' ') + str((ctx != None))))
    finally:
        if hasattr(ctx, 'close'): ctx.close()
    print('DummyContext exited: ok')
    reg = Registry()
    dec_add = reg.register('add')
    dec_add(main__add)
    dec_greet = reg.register('greet')
    dec_greet(main__greet)
    print(((str('Registry names:') + ' ') + str(reg.names())))
    print(((str('dispatch add(3,4):') + ' ') + str(reg.dispatch('add', 3, 4))))
    print(((str('dispatch greet:') + ' ') + str(reg.dispatch('greet', 'celery'))))
    print(((str('dispatch unknown:') + ' ') + str(reg.dispatch('missing', 1))))
    sig = Signal('task_success')
    received = []
    sig.connect(main__on_task_success)
    sig.connect(main__on_task_success2)
    results = sig.send('result_value', 'worker1')
    print(((str('Signal send results count:') + ' ') + str(len(results))))
    print(((str('Signal result[0]:') + ' ') + str(results[0])))
    print(((str('Signal result[1]:') + ' ') + str(results[1])))
    print(((str('Signal received args:') + ' ') + str(received[0])))
    sig.disconnect(main__on_task_success)
    results2 = sig.send('another')
    print(((str('After disconnect count:') + ' ') + str(len(results2))))
    return 0


if __name__ == "__main__":
    main()
