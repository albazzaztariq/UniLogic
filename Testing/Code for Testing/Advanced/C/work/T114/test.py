from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def get_value():
    return 42

def get_name():
    return 'resource'

def main():
    val = get_value()
    try:
        print(val)
    finally:
        if hasattr(val, 'close'): val.close()
    v = get_value()
    try:
        doubled = (v * 2)
        print(doubled)
    finally:
        if hasattr(v, 'close'): v.close()
    name = get_name()
    try:
        print(('got: ' + name))
    finally:
        if hasattr(name, 'close'): name.close()
    a = get_value()
    try:
        b = get_value()
        try:
            print((a + b))
        finally:
            if hasattr(b, 'close'): b.close()
    finally:
        if hasattr(a, 'close'): a.close()
    print('after with')
    return 0


if __name__ == "__main__":
    main()
