from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    f = 3.9
    n = int(f)
    print(n)
    s = str(42)
    print(s)
    g = float(10)
    print(g)
    b1 = bool(1)
    print(b1)
    b2 = bool(0)
    print(b2)
    score = 100
    msg = ('Score: ' + str(score))
    print(msg)
    return 0


if __name__ == "__main__":
    main()
