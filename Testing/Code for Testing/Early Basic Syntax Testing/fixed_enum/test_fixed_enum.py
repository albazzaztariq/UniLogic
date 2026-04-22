from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

from enum import IntEnum

class Priority(IntEnum):
    low = 1
    medium = 2
    high = 3


def main():
    p = Priority.high
    print(p)
    q = Priority.low
    print(q)
    if (p > q):
        print('high > low')
    return 0


if __name__ == "__main__":
    main()
