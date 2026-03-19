from dataclasses import dataclass, field
from math import fmod, pow, sqrt


def main():
    s = sqrt(625.0)
    print(s)
    p = pow(2.0, 10.0)
    print(p)
    m = fmod(17.0, 5.0)
    print(m)
    chained = sqrt(pow(3.0, 4.0))
    print(chained)
    return 0


if __name__ == "__main__":
    main()
