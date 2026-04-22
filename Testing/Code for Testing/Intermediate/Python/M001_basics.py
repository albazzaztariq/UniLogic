from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    x = 10
    y = 20
    sum = (x + y)
    print(sum)
    pi = 3.14
    print(pi)
    name = 'UniLogic'
    print(name)
    flag = True
    print(flag)
    a = 100
    a += 50
    a -= 25
    a *= 2
    print(a)
    mod = (17 % 5)
    print(mod)
    greeting = ('Hello ' + 'World')
    print(greeting)
    MAX = 999
    print(MAX)
    return 0


if __name__ == "__main__":
    main()
