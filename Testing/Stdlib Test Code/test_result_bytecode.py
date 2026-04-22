from dataclasses import dataclass, field
import sys

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def divide(a, b):
    if (b == 0):
        return (False, 'division by zero')
    return (True, (a // b))

def main():
    _r0 = divide(10, 2)
    if not _r0[0]:
        print("error: " + str(_r0[1]), file=sys.stderr)
        sys.exit(1)
    r1 = _r0[1]
    print(r1)
    _r1 = divide(100, 5)
    if not _r1[0]:
        print("error: " + str(_r1[1]), file=sys.stderr)
        sys.exit(1)
    r2 = _r1[1]
    print(r2)
    _r2 = divide(42, 7)
    if not _r2[0]:
        print("error: " + str(_r2[1]), file=sys.stderr)
        sys.exit(1)
    r3 = _r2[1]
    print(r3)
    return 0


if __name__ == "__main__":
    main()
