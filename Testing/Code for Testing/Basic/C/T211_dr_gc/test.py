# @dr types = strict

from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def add_strict(a, b):
    return (a + b)

def greet_strict(who):
    return ('Hello ' + who)

def main():
    result = add_strict(3, 7)
    print(result)
    msg = greet_strict('World')
    print(msg)
    x = 10
    if (x > 5):
        print('strict check passed')
    return 0


if __name__ == "__main__":
    main()
