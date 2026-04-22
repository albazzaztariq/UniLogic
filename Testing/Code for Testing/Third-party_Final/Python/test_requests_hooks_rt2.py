# @dr python_compat = true


def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

HOOKS = ['response', 'error', 'redirect']

def default_hooks():
    global HOOKS
    _dict0 = {}
    result = _dict0
    for event in HOOKS:
        result[event] = []
    return result

def dispatch_hook(key, hooks, hook_data):
    if (not hooks):
        return hook_data
    handler_list = hooks.get(key)
    if handler_list:
        for hook in handler_list:
            new_data = hook(hook_data)
            if (new_data != None):
                hook_data = new_data
    return hook_data

def add_one(x):
    return (x + 1)

def _py_double(x):
    return (x * 2)

def main():
    hooks = default_hooks()
    print(((str('default_hooks keys:') + ' ') + str(list(hooks.keys()))))
    hooks['response'].append(add_one)
    hooks['response'].append(_py_double)
    result = dispatch_hook('response', hooks, 5)
    print(((str('dispatch_hook result:') + ' ') + str(result)))
    result2 = dispatch_hook('error', hooks, 99)
    print(((str('dispatch_hook empty key:') + ' ') + str(result2)))
    result3 = dispatch_hook('response', None, 10)
    print(((str('dispatch_hook None hooks:') + ' ') + str(result3)))
    return 0


if __name__ == "__main__":
    main()
