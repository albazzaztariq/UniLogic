from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def greet(who, greeting='Hello'):
    return ((greeting + ' ') + who)

def main():
    def add_offset(x):
        offset = 100
        return (x + offset)
    
    def make_label(n):
        return ('item_' + str(n))
    
    def doubler(y):
        return (y * 2)
    
    def calc(x):
        return (doubler(x) + 1)
    
    print(add_offset(5))
    print(make_label(42))
    print(calc(10))
    print(greet('World', 'Hello'))
    print(greet('World', 'Hi'))
    return 0


if __name__ == "__main__":
    main()
