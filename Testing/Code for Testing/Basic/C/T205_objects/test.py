from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class Counter:
    def __init__(self, count=0):
        self.count = count
    def get_count(self):
        return self.count
    def increment(self):
        self.count = (self.count + 1)
        return self.count

class DoubleCounter(Counter):
    def __init__(self, count=0):
        super().__init__(count)
    def double_increment(self):
        self.count = (self.count + 2)
        return self.count


def main():
    c = Counter()
    c.count = 0
    print(c.get_count())
    print(c.increment())
    print(c.increment())
    print(c.get_count())
    dc = DoubleCounter()
    dc.count = 10
    print(dc.get_count())
    print(dc.double_increment())
    return 0


if __name__ == "__main__":
    main()
