from dataclasses import dataclass, field
import sys

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def check(val):
    if (val < 0):
        print((-1))
        sys.exit(1)
    return (val * 2)

def main():
    result = check(5)
    print(result)
    sys.exit(0)
    return 0


if __name__ == "__main__":
    main()
