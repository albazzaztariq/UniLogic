"""Sorting algorithms — bubble, selection, insertion sort with comparison counting."""


def bubble_sort(arr: list, count: int) -> int:
    comparisons: int = 0
    i: int = 0
    while i < count - 1:
        j: int = 0
        while j < count - 1 - i:
            comparisons = comparisons + 1
            if arr[j] > arr[j + 1]:
                temp: int = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = temp
            j = j + 1
        i = i + 1
    return comparisons


def selection_sort(arr: list, count: int) -> int:
    comparisons: int = 0
    i: int = 0
    while i < count - 1:
        min_idx: int = i
        j: int = i + 1
        while j < count:
            comparisons = comparisons + 1
            if arr[j] < arr[min_idx]:
                min_idx = j
            j = j + 1
        if min_idx != i:
            temp: int = arr[i]
            arr[i] = arr[min_idx]
            arr[min_idx] = temp
        i = i + 1
    return comparisons


def insertion_sort(arr: list, count: int) -> int:
    comparisons: int = 0
    i: int = 1
    while i < count:
        key: int = arr[i]
        j: int = i - 1
        done: bool = False
        while j >= 0 and not done:
            comparisons = comparisons + 1
            if arr[j] > key:
                arr[j + 1] = arr[j]
                j = j - 1
            else:
                done = True
        arr[j + 1] = key
        i = i + 1
    return comparisons


def array_copy(src: list, dst: list, count: int) -> None:
    i: int = 0
    while i < count:
        dst[i] = src[i]
        i = i + 1


def is_sorted(arr: list, count: int) -> bool:
    i: int = 0
    while i < count - 1:
        if arr[i] > arr[i + 1]:
            return False
        i = i + 1
    return True


def print_array(arr: list, count: int) -> None:
    result: str = ""
    i: int = 0
    while i < count:
        if i > 0:
            result = result + " "
        result = result + str(arr[i])
        i = i + 1
    print(result)


def array_sum(arr: list, count: int) -> int:
    total: int = 0
    i: int = 0
    while i < count:
        total = total + arr[i]
        i = i + 1
    return total


def test_sort(name: str, original: list, work: list, count: int, sort_type: int) -> None:
    array_copy(original, work, count)
    comparisons: int = 0
    if sort_type == 1:
        comparisons = bubble_sort(work, count)
    if sort_type == 2:
        comparisons = selection_sort(work, count)
    if sort_type == 3:
        comparisons = insertion_sort(work, count)
    print(name + ": comps=" + str(comparisons) + " sorted=" + str(is_sorted(work, count)) + " sum=" + str(array_sum(work, count)))


def main() -> int:
    data: list = [64, 34, 25, 12, 22, 11, 90, 45, 0, 0]
    count: int = 8
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

    print("--- Dataset 1 ---")
    print_array(data, count)
    test_sort("Bubble", data, work, count, 1)
    test_sort("Selection", data, work, count, 2)
    test_sort("Insertion", data, work, count, 3)

    data2: list = [5, 1, 4, 2, 8, 3, 7, 6, 9, 10]
    count2: int = 10
    print("--- Dataset 2 ---")
    print_array(data2, count2)
    test_sort("Bubble", data2, work, count2, 1)
    test_sort("Selection", data2, work, count2, 2)
    test_sort("Insertion", data2, work, count2, 3)

    # Already sorted
    sorted_data: list = [1, 2, 3, 4, 5, 6, 7, 8, 0, 0]
    count3: int = 8
    print("--- Already sorted ---")
    test_sort("Bubble", sorted_data, work, count3, 1)
    test_sort("Insertion", sorted_data, work, count3, 3)

    # Reverse sorted
    rev_data: list = [8, 7, 6, 5, 4, 3, 2, 1, 0, 0]
    print("--- Reverse sorted ---")
    test_sort("Bubble", rev_data, work, count3, 1)
    test_sort("Insertion", rev_data, work, count3, 3)

    # Single element
    single: list = [42, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    print("--- Single ---")
    test_sort("Bubble", single, work, 1, 1)

    return 0


if __name__ == "__main__":
    main()
