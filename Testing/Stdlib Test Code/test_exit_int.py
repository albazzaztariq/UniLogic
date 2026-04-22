from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def validate(x):
    if (x < 0):
        return (-1)
    return x

def main():
    val = validate(10)
    if (val < 0):
        print(0)
        return 1
    print(val)
    return 0


if __name__ == "__main__":
    main()
