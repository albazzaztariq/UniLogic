from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

from enum import IntEnum

class Status(IntEnum):
    pending = 0
    active = 1
    closed = 2

class Level(IntEnum):
    low = 10
    medium = 20
    high = 30

MAX_ITEMS = 50
DOUBLED = (MAX_ITEMS * 2)
VERSION = '1.0'
counter = 0
prefix = 'item_'

def increment_counter():
    global counter
    counter = (counter + 1)
    return counter

def main():
    global counter, prefix
    print(MAX_ITEMS)
    print(DOUBLED)
    print(VERSION)
    print(counter)
    print(prefix)
    print(increment_counter())
    print(increment_counter())
    print(increment_counter())
    s = Status.active
    print(s)
    total = ((Status.pending + Status.active) + Status.closed)
    print(total)
    if (s == Status.active):
        print('is active')
    lev = Level.medium
    match lev:
        case 10:
            print('low')
        case 20:
            print('medium')
        case 30:
            print('high')
    return 0


if __name__ == "__main__":
    main()
