from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

MAX_SIZE = 1024
PI = 3.14159
GREETING = 'hello'
PRIMES = [2, 3, 5, 7, 11]

def double_max():
    global MAX_SIZE
    return (MAX_SIZE * 2)

def main():
    global GREETING, MAX_SIZE, PI, PRIMES
    print(MAX_SIZE)
    print(PI)
    print(GREETING)
    print(PRIMES[0])
    print(PRIMES[4])
    print(double_max())
    MAX_SIZE = 2048
    print(MAX_SIZE)
    return 0


if __name__ == "__main__":
    main()
