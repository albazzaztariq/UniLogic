from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def array_sum(data, count):
    total = 0
    i = 0
    while (i < count):
        total = (total + data[i])
        i = (i + 1)
    return total

def array_min(data, count):
    result = data[0]
    i = 1
    while (i < count):
        if (data[i] < result):
            result = data[i]
        i = (i + 1)
    return result

def array_max(data, count):
    result = data[0]
    i = 1
    while (i < count):
        if (data[i] > result):
            result = data[i]
        i = (i + 1)
    return result

def array_range(data, count):
    return (array_max(data, count) - array_min(data, count))

def bubble_sort(data, count):
    i = 0
    while (i < (count - 1)):
        j = 0
        while (j < ((count - 1) - i)):
            if (data[j] > data[(j + 1)]):
                temp = data[j]
                data[j] = data[(j + 1)]
                data[(j + 1)] = temp
            j = (j + 1)
        i = (i + 1)

def median_val(data, count):
    bubble_sort(data, count)
    return data[(count // 2)]

def main():
    data = [23, 7, 42, 15, 8, 31, 19, 3, 50, 12]
    count = 10
    print(array_sum(data, count))
    print(array_min(data, count))
    print(array_max(data, count))
    print(array_range(data, count))
    print(median_val(data, count))
    print(data[0])
    print(data[9])
    data2 = [100, 200, 300, 400, 500]
    count2 = 5
    print(array_sum(data2, count2))
    print(array_min(data2, count2))
    print(array_max(data2, count2))
    return 0


if __name__ == "__main__":
    main()
