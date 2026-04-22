from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def twice(x):
    return (x * 2)

def add_one(x):
    return (x + 1)

def flip_sign(x):
    return (0 - x)

def main():
    a = twice(5)
    print(a)
    b = add_one(twice(5))
    print(b)
    c = flip_sign(add_one(twice(3)))
    print(c)
    d = add_one(10)
    print(d)
    return 0


if __name__ == "__main__":
    main()
