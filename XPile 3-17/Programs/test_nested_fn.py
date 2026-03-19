from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def outer():
    def add_ten(x):
        return (x + 10)
    
    def twice(x):
        return (x * 2)
    
    a = add_ten(5)
    b = twice(a)
    print(a)
    print(b)
    return b

def main():
    def greet():
        return 'hello from nested'
    
    result = outer()
    print(result)
    msg = greet()
    print(msg)
    return 0


if __name__ == "__main__":
    main()
