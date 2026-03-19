from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class Priority:
    low = 1
    medium = 2
    high = 3

class Color:
    red = 10
    green = 20
    blue = 30


def main():
    p = Priority.high
    print(p)
    total = ((Priority.low + Priority.medium) + Priority.high)
    print(total)
    if (p == Priority.high):
        print(1)
    c = Color.green
    print(c)
    val = Priority.medium
    match val:
        case 1:
            print(100)
        case 2:
            print(200)
        case 3:
            print(300)
    return 0


if __name__ == "__main__":
    main()
