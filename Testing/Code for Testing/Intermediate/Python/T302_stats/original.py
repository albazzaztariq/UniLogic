"""Basic statistics functions — mean, min, max, variance."""


def array_sum(data: list, count: int) -> int:
    total: int = 0
    i: int = 0
    while i < count:
        total = total + data[i]
        i = i + 1
    return total


def array_min(data: list, count: int) -> int:
    result: int = data[0]
    i: int = 1
    while i < count:
        if data[i] < result:
            result = data[i]
        i = i + 1
    return result


def array_max(data: list, count: int) -> int:
    result: int = data[0]
    i: int = 1
    while i < count:
        if data[i] > result:
            result = data[i]
        i = i + 1
    return result


def array_range(data: list, count: int) -> int:
    return array_max(data, count) - array_min(data, count)


def bubble_sort(data: list, count: int) -> None:
    i: int = 0
    while i < count - 1:
        j: int = 0
        while j < count - 1 - i:
            if data[j] > data[j + 1]:
                temp: int = data[j]
                data[j] = data[j + 1]
                data[j + 1] = temp
            j = j + 1
        i = i + 1


def median_val(data: list, count: int) -> int:
    bubble_sort(data, count)
    return data[count // 2]


def main() -> int:
    data: list = [23, 7, 42, 15, 8, 31, 19, 3, 50, 12]
    count: int = 10

    print(array_sum(data, count))
    print(array_min(data, count))
    print(array_max(data, count))
    print(array_range(data, count))

    # Median (sorts the array)
    print(median_val(data, count))

    # After sort, check first and last
    print(data[0])
    print(data[9])

    # Second dataset
    data2: list = [100, 200, 300, 400, 500]
    count2: int = 5
    print(array_sum(data2, count2))
    print(array_min(data2, count2))
    print(array_max(data2, count2))

    return 0


if __name__ == "__main__":
    main()
