from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

MAX = 100
LABEL = 'test'

def main():
    print(MAX)
    print(LABEL)
    return 0


if __name__ == "__main__":
    main()
