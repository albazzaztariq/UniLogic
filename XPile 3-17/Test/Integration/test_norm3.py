from dataclasses import dataclass, field
from math import sqrt


def main():
    x = 5
    x = (x + 10)
    x = (x * 2)
    x = (x - 1)
    print(x)
    c = 0
    c = (c + 1)
    c = (c + 1)
    c = (c + 1)
    c = (c - 1)
    print(c)
    _t0 = sqrt(256.0)
    _t1 = sqrt(_t0)
    v = sqrt(_t1)
    print(v)
    raw = 97
    _t2 = float(raw)
    d = float(_t2)
    print(d)
    a = 10
    b = 20
    c2 = 30
    d2 = 40
    _t3 = ((a > 0) and (b > 0))
    _t4 = (_t3 and (c2 > 0))
    if (_t4 and (d2 > 0)):
        print(1)
    _t5 = ((a > 0) and (b > 100))
    if (_t5 and (c2 > 0)):
        print(999)
    print(0)
    return 0


if __name__ == "__main__":
    main()
