from dataclasses import dataclass, field
from math import sqrt
def puts(s): print(s); return 0

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    val = 144.0
    result = sqrt(val)
    print(result)
    puts('Hello from FFI!')
    return 0


if __name__ == "__main__":
    main()
