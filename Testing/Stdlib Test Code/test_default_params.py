from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def greet(name, greeting='Hello'):
    return '{greeting}, {name}!'

def add(a, b=10):
    return (a + b)

def main():
    print(greet('World', 'Hello'))
    print(greet('World', 'Hi'))
    print(add(5, 10))
    print(add(5, 20))
    return 0


if __name__ == "__main__":
    main()
