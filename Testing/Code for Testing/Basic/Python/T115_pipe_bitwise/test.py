# @dr safety = checked

from dataclasses import dataclass, field
import sys

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def triple(x):
    return (x * 3)

def add_ten(x):
    return (x + 10)

def flip_sign(x):
    return (0 - x)

def main():
    a = triple(5)
    print(a)
    b = add_ten(triple(5))
    print(b)
    c = flip_sign(add_ten(triple(2)))
    print(c)
    x = 10
    if not ((x > 0)):
        print(f"killswitch failed at line 32", file=sys.stderr)
        sys.exit(1)
    print('killswitch passed')
    if not ((x == 10)):
        print(f"killswitch failed at line 35", file=sys.stderr)
        sys.exit(1)
    print('killswitch 2 passed')
    r1 = (255 & 15)
    print(r1)
    r2 = (240 | 15)
    print(r2)
    r3 = (255 ^ 15)
    print(r3)
    r4 = (1 << 4)
    print(r4)
    r5 = (256 >> 4)
    print(r5)
    r6 = (~0)
    print(r6)
    return 0


if __name__ == "__main__":
    main()
