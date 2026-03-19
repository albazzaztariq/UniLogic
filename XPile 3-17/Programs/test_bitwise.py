from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    a = 255
    b = 15
    r1 = (a & b)
    print(r1)
    r2 = (240 | 15)
    print(r2)
    r3 = (255 ^ 15)
    print(r3)
    r4 = (~0)
    print(r4)
    r5 = (1 << 4)
    print(r5)
    r6 = (256 >> 4)
    print(r6)
    return 0


if __name__ == "__main__":
    main()
