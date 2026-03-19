# @dr memory = refcount

from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

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
