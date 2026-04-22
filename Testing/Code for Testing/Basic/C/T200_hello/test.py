from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    print('Hello, UniLogic!')
    x = 42
    print(x)
    pi = 3.14
    print(pi)
    name = 'UniLogic'
    print(name)
    flag = True
    print(flag)
    sum = (10 + 20)
    print(sum)
    greeting = ('Hello ' + 'World')
    print(greeting)
    a = 5
    b = 10
    c = (a + b)
    print(c)
    return 0


if __name__ == "__main__":
    main()
