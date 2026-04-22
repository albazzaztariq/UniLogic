from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def minmax(a, b):
    if (a < b):
        return (a, b)
    return (b, a)

def main():
    lo, hi = minmax(10, 3)
    print(lo)
    print(hi)
    return 0


if __name__ == "__main__":
    main()
