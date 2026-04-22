from dataclasses import dataclass, field
from math import sqrt

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    counter = 10
    counter += 5
    counter *= 2
    i = 0
    i += 1
    i += 1
    i -= 1
    print(counter)
    print(i)
    val = 144.0
    deep = sqrt(sqrt(val))
    print(deep)
    x = 42
    y = float(float(x))
    print(y)
    a = 1
    b = 2
    c = 3
    if (((a > 0) and (b > 0)) and (c > 0)):
        print(a)
    return 0


if __name__ == "__main__":
    main()
