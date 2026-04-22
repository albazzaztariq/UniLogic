from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

from enum import IntEnum

class Priority(IntEnum):
    low = 1
    medium = 2
    high = 3

class Color(IntEnum):
    red = 10
    green = 20
    blue = 30

VERSION = 1
APP = 'TestApp'

def main():
    print(VERSION)
    print(APP)
    p = Priority.high
    print(p)
    c = Color.green
    print(c)
    if (p == 3):
        print('high priority confirmed')
    return 0


if __name__ == "__main__":
    main()
