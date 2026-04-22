from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

MAX = 100
VERSION = '1.0.0'
PI = 3.14

def main():
    print(MAX)
    print(VERSION)
    print(PI)
    return 0


if __name__ == "__main__":
    main()
