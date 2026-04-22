from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    squares = [(x * x) for x in range(1, 6)]
    print(squares[0])
    print(squares[4])
    return 0


if __name__ == "__main__":
    main()
