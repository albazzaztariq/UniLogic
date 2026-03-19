from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    total = 0
    # parallel for -- using multiprocessing.Pool
    import multiprocessing
    def _parallel_body_0(i):
        total = (total + i)
    with multiprocessing.Pool() as _pool:
        _pool.map(_parallel_body_0, range(100))
    print(total)
    return 0


if __name__ == "__main__":
    main()
