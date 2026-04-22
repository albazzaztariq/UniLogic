from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

@dataclass
class StatusFlags:
    ready: int = 0  # bit field: 1 bits
    fault: int = 0  # bit field: 1 bits
    mode: int = 0  # bit field: 3 bits
    priority: int = 0  # bit field: 3 bits


def main():
    flags = StatusFlags()
    flags.ready = 1
    flags.fault = 0
    flags.mode = 5
    flags.priority = 3
    print(flags.ready)
    print(flags.fault)
    print(flags.mode)
    print(flags.priority)
    return 0


if __name__ == "__main__":
    main()
