from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def countdown(n):
    while (n > 0):
        yield n
        n -= 1

def doubles(n):
    i = 1
    while (i <= n):
        yield (i * 2)
        i += 1

def main():
    for val in countdown(5):
        print(val)
    for d in doubles(4):
        print(d)
    return 0


if __name__ == "__main__":
    main()
