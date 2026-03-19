# @dr concurrency = threaded

from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

shared_counter = 0

def worker():
    global shared_counter
    i = 0
    while (i < 1000):
        lock(counter_lock)
        shared_counter = (shared_counter + 1)
        unlock(counter_lock)
        i = (i + 1)

def main():
    global shared_counter
    spawn(worker())
    spawn(worker())
    wait()
    print(shared_counter)
    return 0


if __name__ == "__main__":
    main()
