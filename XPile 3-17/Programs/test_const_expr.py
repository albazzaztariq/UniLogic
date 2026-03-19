from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

MAX = 100
DOUBLE_MAX = (MAX * 2)
OFFSET = (MAX + 1)
PI = 3.14159
TAU = (PI * 2)
GREETING = ('Hello' + ' World')
FLAG = True

def main():
    print(MAX)
    print(DOUBLE_MAX)
    print(OFFSET)
    print(TAU)
    print(GREETING)
    print(FLAG)
    x = (MAX + 10)
    print(x)
    y = (DOUBLE_MAX - OFFSET)
    print(y)
    return 0


if __name__ == "__main__":
    main()
