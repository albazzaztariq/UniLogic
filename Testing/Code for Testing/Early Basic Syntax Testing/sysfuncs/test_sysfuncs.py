from dataclasses import dataclass, field
import random
import time

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    t = int(time.time())
    if (t > 0):
        print('time ok')
    c = time.time_ns()
    if (c > 0):
        print('clock ok')
    random.seed(42)
    r = random.random()
    if (r >= 0.0):
        if (r <= 1.0):
            print('random ok')
    ri = random.randint(1, 10)
    if (ri >= 1):
        if (ri <= 10):
            print('random_int ok')
    return 0


if __name__ == "__main__":
    main()
