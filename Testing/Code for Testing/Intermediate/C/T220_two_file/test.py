from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def clamp(val, lo, hi):
    if (val < lo):
        return lo
    if (val > hi):
        return hi
    return val

def abs_val(x):
    if (x < 0):
        return (0 - x)
    return x

def max_of(a, b):
    if (a > b):
        return a
    return b

def repeat_string(s, n):
    result = ''
    i = 0
    while (i < n):
        result = (result + s)
        i = (i + 1)
    return result

def main():
    print(clamp(5, 0, 10))
    print(clamp((-3), 0, 10))
    print(clamp(15, 0, 10))
    print(abs_val(7))
    print(abs_val((-12)))
    print(max_of(3, 8))
    print(max_of(20, 5))
    print(repeat_string('ab', 3))
    result = clamp(abs_val((-25)), 0, 20)
    print(result)
    return 0


if __name__ == "__main__":
    main()
