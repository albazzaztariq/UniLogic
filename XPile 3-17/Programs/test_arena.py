# @dr memory = arena

from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

# @dr memory = arena -- arena allocation simulated
# Python uses reference counting; this mode disables explicit del calls
# and relies on scope-based cleanup via context managers
__ul_arena_refs = []

def __ul_arena_alloc(obj):
    __ul_arena_refs.append(obj)
    return obj

def __ul_arena_free():
    __ul_arena_refs.clear()

import atexit
atexit.register(__ul_arena_free)

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def allocate_many():
    total = 0
    i = 0
    while (i < 1000):
        s = ('item ' + str(i))
        total = (total + len(s))
        i = (i + 1)
    return total

def main():
    result = allocate_many()
    print(result)
    return 0


if __name__ == "__main__":
    main()
