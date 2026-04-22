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

class Signal:
    def __init__(self, name):
        self.name = name
        self._receivers = []
    def connect(self, receiver):
        self._receivers.append(receiver)
    def send(self, *_va_args):
        results = []
        for r in self._receivers:
            results.append(r(*_va_args))
        return results
    def __repr__(self):
        return (("<Signal '" + str(self.name)) + "'>")

class Namespace:
    def __init__(self):
        _dict0 = {}
        self._signals = _dict0
    def signal(self, name):
        if (not (name in self._signals)):
            self._signals[name] = Signal(name)
        return self._signals[name]


def main__on_template_rendered(data):
    received.append(('rendered:' + data))
    return ('rendered:' + data)

def main():
    ns = Namespace()
    template_rendered = ns.signal('template-rendered')
    request_started = ns.signal('request-started')
    got_request_exception = ns.signal('got-request-exception')
    print(((str('template_rendered:') + ' ') + str(template_rendered)))
    print(((str('request_started:') + ' ') + str(request_started)))
    received = []
    template_rendered.connect(main__on_template_rendered)
    template_rendered.send('index.html')
    print(((str('received:') + ' ') + str(received)))
    print(((str('same signal:') + ' ') + str((ns.signal('template-rendered') == template_rendered))))
    return 0


if __name__ == "__main__":
    main()
