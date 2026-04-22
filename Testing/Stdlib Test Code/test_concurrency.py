# @dr concurrency = threaded

from dataclasses import dataclass, field
import threading

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

__ul_threads = []

def worker(id):
    print(id)

def main():
    _tid_0 = threading.Thread(target=worker, args=(1,))
    _tid_0.start()
    __ul_threads.append(_tid_0)
    (len(__ul_threads) - 1)
    _tid_1 = threading.Thread(target=worker, args=(2,))
    _tid_1.start()
    __ul_threads.append(_tid_1)
    (len(__ul_threads) - 1)
    _tid_2 = threading.Thread(target=worker, args=(3,))
    _tid_2.start()
    __ul_threads.append(_tid_2)
    (len(__ul_threads) - 1)
    for _t in __ul_threads: _t.join()
    __ul_threads.clear()
    None
    return 0


if __name__ == "__main__":
    main()
