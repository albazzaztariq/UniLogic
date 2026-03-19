from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    count = 0
    while True:
        count = (count + 1)
        if not ((count < 5)):
            break
    print(count)
    x = 10
    while True:
        x = (x + 1)
        if not ((x < 5)):
            break
    print(x)
    n = 0
    while True:
        n = (n + 1)
        if (n == 3):
            break
        if not ((n < 10)):
            break
    print(n)
    return 0


if __name__ == "__main__":
    main()
