# @dr concurrency = threaded

from dataclasses import dataclass, field


def worker(id):
    print(id)

def main():
    spawn(worker(1))
    spawn(worker(2))
    spawn(worker(3))
    wait()
    return 0


if __name__ == "__main__":
    main()
