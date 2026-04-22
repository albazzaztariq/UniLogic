from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

# @packed - no padding (packed struct)
@dataclass
class PackedData:
    nibble_hi: int = 0
    nibble_lo: int = 0
    byte_val: int = 0


def main():
    d = PackedData(nibble_hi=15, nibble_lo=9, byte_val=200)
    print(d.nibble_hi)
    print(d.nibble_lo)
    print(d.byte_val)
    return 0


if __name__ == "__main__":
    main()
