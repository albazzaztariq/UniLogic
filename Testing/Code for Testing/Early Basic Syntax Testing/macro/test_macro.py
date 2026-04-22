from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    a = ((5) * 2)
    print(a)
    b = ((3) + (7))
    print(b)
    c = ((6) * (6))
    print(c)
    d = ((((2) * 2)) + (((3) * (3))))
    print(d)
    return 0


if __name__ == "__main__":
    main()
