from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def get_resource():
    return 42

def main():
    r = get_resource()
    try:
        print(r)
    finally:
        if hasattr(r, 'close'): r.close()
    val = get_resource()
    try:
        doubled = (val * 2)
        print(doubled)
    finally:
        if hasattr(val, 'close'): val.close()
    print(1)
    return 0


if __name__ == "__main__":
    main()
