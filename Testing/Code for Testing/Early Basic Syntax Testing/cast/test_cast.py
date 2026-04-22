from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    a = 42
    b = float(a)
    print(b)
    c = 3.7
    d = int(c)
    print(d)
    e = 123
    f = str(e)
    print(f)
    g = True
    h = int(g)
    print(h)
    i = 0
    j = bool(i)
    print(j)
    k = 1
    l = bool(k)
    print(l)
    return 0


if __name__ == "__main__":
    main()
