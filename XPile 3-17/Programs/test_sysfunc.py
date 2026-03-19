from dataclasses import dataclass, field
import os
import random
import time

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    t = int(time.time())
    if (t > 0):
        print(1)
    c = time.time_ns()
    if (c > 0):
        print(1)
    random.seed(42)
    r1 = random.randint(1, 100)
    if ((r1 >= 1) and (r1 <= 100)):
        print(1)
    random.seed(42)
    f = random.random()
    if ((f >= 0.0) and (f < 1.0)):
        print(1)
    path = os.environ.get('PATH', '')
    if (len(path) > 0):
        print(1)
    return 0


if __name__ == "__main__":
    main()
