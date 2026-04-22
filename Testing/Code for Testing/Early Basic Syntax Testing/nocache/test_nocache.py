from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    sensor_value = 0  # nocache (volatile)
    sensor_value = 42
    print(sensor_value)
    counter = 10  # nocache (volatile)
    counter += 5
    print(counter)
    return 0


if __name__ == "__main__":
    main()
