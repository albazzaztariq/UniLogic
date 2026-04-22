from dataclasses import dataclass, field
import sys

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def level1(x):
    if (x < 0):
        return (False, 'negative input')
    return (True, (x * 10))

def level2(x):
    _r0 = level1(x)
    if not _r0[0]:
        return _r0
    a = _r0[1]
    return (True, (a + 1))

def level3(x):
    _r1 = level2(x)
    if not _r1[0]:
        return _r1
    b = _r1[1]
    return (True, (b + 2))

def main():
    _r2 = level3(5)
    if not _r2[0]:
        print("error: " + str(_r2[1]), file=sys.stderr)
        sys.exit(1)
    r1 = _r2[1]
    print(r1)
    _r3 = level3(0)
    if not _r3[0]:
        print("error: " + str(_r3[1]), file=sys.stderr)
        sys.exit(1)
    r2 = _r3[1]
    print(r2)
    _r4 = level3((-1))
    if not _r4[0]:
        print("error: " + str(_r4[1]), file=sys.stderr)
        sys.exit(1)
    r3 = _r4[1]
    print(r3)
    return 0


if __name__ == "__main__":
    main()
