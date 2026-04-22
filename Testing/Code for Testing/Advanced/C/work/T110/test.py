from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def compute(x):
    return (x * 10)

def main():
    a = compute(5)
    print(a)
    b = compute(3)
    print(b)
    total = (a + b)
    print(total)
    return 0


if __name__ == "__main__":
    main()
