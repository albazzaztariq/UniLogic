from dataclasses import dataclass, field
import sys

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def divide(a, b):
    if (b == 0):
        return (False, 'division by zero')
    return (True, (a // b))

def safe_calc(x, y):
    _r0 = divide(x, y)
    if not _r0[0]:
        return _r0
    result = _r0[1]
    return (True, (result * 2))

def main():
    _r1 = divide(10, 2)
    if not _r1[0]:
        print("error: " + str(_r1[1]), file=sys.stderr)
        sys.exit(1)
    a = _r1[1]
    print(a)
    _r2 = safe_calc(20, 4)
    if not _r2[0]:
        print("error: " + str(_r2[1]), file=sys.stderr)
        sys.exit(1)
    b = _r2[1]
    print(b)
    _r3 = divide(5, 0)
    if not _r3[0]:
        print("error: " + str(_r3[1]), file=sys.stderr)
        sys.exit(1)
    c = _r3[1]
    print(c)
    return 0


if __name__ == "__main__":
    main()
