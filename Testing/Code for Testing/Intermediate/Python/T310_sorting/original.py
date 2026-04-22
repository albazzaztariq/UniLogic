"""Sorting algorithms suite — bubble, selection, insertion, merge, quick sort.
Comparison counting, multiple test arrays, validation."""


def array_copy(src: list, dst: list, count: int) -> None:
    i: int = 0
    while i < count:
        dst[i] = src[i]
        i = i + 1


def array_eq(a: list, b: list, count: int) -> bool:
    i: int = 0
    while i < count:
        if a[i] != b[i]:
            return False
        i = i + 1
    return True


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


def merge_arrays(arr: list, temp: list, left: int, mid: int, right: int) -> int:
    comparisons: int = 0
    i: int = left
    j: int = mid + 1
    k: int = left
    while i <= mid and j <= right:
        comparisons = comparisons + 1
        if arr[i] <= arr[j]:
            temp[k] = arr[i]
            i = i + 1
        else:
            temp[k] = arr[j]
            j = j + 1
        k = k + 1
    while i <= mid:
        temp[k] = arr[i]
        i = i + 1
        k = k + 1
    while j <= right:
        temp[k] = arr[j]
        j = j + 1
        k = k + 1
    i = left
    while i <= right:
        arr[i] = temp[i]
        i = i + 1
    return comparisons


def merge_sort_helper(arr: list, temp: list, left: int, right: int) -> int:
    comparisons: int = 0
    if left < right:
        mid: int = (left + right) // 2
        comparisons = comparisons + merge_sort_helper(arr, temp, left, mid)
        comparisons = comparisons + merge_sort_helper(arr, temp, mid + 1, right)
        comparisons = comparisons + merge_arrays(arr, temp, left, mid, right)
    return comparisons


def merge_sort(arr: list, count: int) -> int:
    temp: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    return merge_sort_helper(arr, temp, 0, count - 1)


def partition(arr: list, low: int, high: int, comp_count: list) -> int:
    pivot: int = arr[high]
    i: int = low - 1
    j: int = low
    while j < high:
        comp_count[0] = comp_count[0] + 1
        if arr[j] <= pivot:
            i = i + 1
            temp: int = arr[i]
            arr[i] = arr[j]
            arr[j] = temp
        j = j + 1
    temp2: int = arr[i + 1]
    arr[i + 1] = arr[high]
    arr[high] = temp2
    return i + 1


def quick_sort_helper(arr: list, low: int, high: int, comp_count: list) -> None:
    if low < high:
        pi: int = partition(arr, low, high, comp_count)
        quick_sort_helper(arr, low, pi - 1, comp_count)
        quick_sort_helper(arr, pi + 1, high, comp_count)


def quick_sort(arr: list, count: int) -> int:
    comp_count: list = [0]
    quick_sort_helper(arr, 0, count - 1, comp_count)
    return comp_count[0]


def shell_sort(arr: list, count: int) -> int:
    comparisons: int = 0
    gap: int = count // 2
    while gap > 0:
        i: int = gap
        while i < count:
            temp: int = arr[i]
            j: int = i
            done: bool = False
            while j >= gap and not done:
                comparisons = comparisons + 1
                if arr[j - gap] > temp:
                    arr[j] = arr[j - gap]
                    j = j - gap
                else:
                    done = True
            arr[j] = temp
            i = i + 1
        gap = gap // 2
    return comparisons


def counting_sort(arr: list, count: int, max_val: int) -> int:
    operations: int = 0
    counts: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    i: int = 0
    while i < count:
        counts[arr[i]] = counts[arr[i]] + 1
        operations = operations + 1
        i = i + 1
    pos: int = 0
    val: int = 0
    while val <= max_val:
        j: int = 0
        while j < counts[val]:
            arr[pos] = val
            pos = pos + 1
            operations = operations + 1
            j = j + 1
        val = val + 1
    return operations


def reverse_array(arr: list, count: int) -> None:
    left: int = 0
    right: int = count - 1
    while left < right:
        temp: int = arr[left]
        arr[left] = arr[right]
        arr[right] = temp
        left = left + 1
        right = right - 1


def find_min(arr: list, count: int) -> int:
    result: int = arr[0]
    i: int = 1
    while i < count:
        if arr[i] < result:
            result = arr[i]
        i = i + 1
    return result


def find_max(arr: list, count: int) -> int:
    result: int = arr[0]
    i: int = 1
    while i < count:
        if arr[i] > result:
            result = arr[i]
        i = i + 1
    return result


def array_sum(arr: list, count: int) -> int:
    total: int = 0
    i: int = 0
    while i < count:
        total = total + arr[i]
        i = i + 1
    return total


def count_inversions(arr: list, count: int) -> int:
    inv: int = 0
    i: int = 0
    while i < count:
        j: int = i + 1
        while j < count:
            if arr[i] > arr[j]:
                inv = inv + 1
            j = j + 1
        i = i + 1
    return inv


def generate_test_array(seed: int, count: int, arr: list) -> None:
    val: int = seed
    i: int = 0
    while i < count:
        val = (val * 1103515245 + 12345) % 100000
        arr[i] = val % 100
        i = i + 1


def test_sort_algorithm(name: str, original: list, work: list, count: int, sort_type: int) -> None:
    array_copy(original, work, count)
    print("--- " + name + " ---")
    print("Before: ")
    print_array(work, count)
    inversions: int = count_inversions(work, count)
    print("Inversions: " + str(inversions))
    comparisons: int = 0
    if sort_type == 1:
        comparisons = bubble_sort(work, count)
    if sort_type == 2:
        comparisons = selection_sort(work, count)
    if sort_type == 3:
        comparisons = insertion_sort(work, count)
    if sort_type == 4:
        comparisons = merge_sort(work, count)
    if sort_type == 5:
        comparisons = quick_sort(work, count)
    if sort_type == 6:
        comparisons = shell_sort(work, count)
    print("After: ")
    print_array(work, count)
    if is_sorted(work, count):
        print("Sorted: yes")
    else:
        print("Sorted: no")
    print("Comparisons: " + str(comparisons))
    print("Sum: " + str(array_sum(work, count)))
    print("Min: " + str(find_min(work, count)))
    print("Max: " + str(find_max(work, count)))


def test_with_dataset(label: str, data: list, count: int) -> None:
    print("====== " + label + " ======")
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    test_sort_algorithm("Bubble Sort", data, work, count, 1)
    test_sort_algorithm("Selection Sort", data, work, count, 2)
    test_sort_algorithm("Insertion Sort", data, work, count, 3)
    test_sort_algorithm("Merge Sort", data, work, count, 4)
    test_sort_algorithm("Quick Sort", data, work, count, 5)
    test_sort_algorithm("Shell Sort", data, work, count, 6)


def test_counting_sort() -> None:
    print("====== Counting Sort ======")
    arr: list = [4, 2, 7, 1, 3, 5, 0, 6, 2, 4, 1, 3, 7, 5, 0]
    count: int = 15
    print("Before: ")
    print_array(arr, count)
    ops: int = counting_sort(arr, count, 7)
    print("After: ")
    print_array(arr, count)
    if is_sorted(arr, count):
        print("Sorted: yes")
    else:
        print("Sorted: no")
    print("Operations: " + str(ops))


def test_reverse() -> None:
    print("====== Reverse ======")
    arr: list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    count: int = 10
    print("Before: ")
    print_array(arr, count)
    reverse_array(arr, count)
    print("After: ")
    print_array(arr, count)
    reverse_array(arr, count)
    print("Reversed back: ")
    print_array(arr, count)


def test_already_sorted() -> None:
    print("====== Already Sorted ======")
    arr: list = [1, 2, 3, 4, 5, 6, 7, 8, 0, 0]
    count: int = 8
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    array_copy(arr, work, count)
    comps: int = bubble_sort(work, count)
    print("Bubble on sorted: " + str(comps) + " comparisons")
    if is_sorted(work, count):
        print("Still sorted: yes")
    array_copy(arr, work, count)
    comps = insertion_sort(work, count)
    print("Insertion on sorted: " + str(comps) + " comparisons")


def test_reverse_sorted() -> None:
    print("====== Reverse Sorted ======")
    arr: list = [8, 7, 6, 5, 4, 3, 2, 1, 0, 0]
    count: int = 8
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    array_copy(arr, work, count)
    comps: int = bubble_sort(work, count)
    print("Bubble on reversed: " + str(comps) + " comparisons")
    print_array(work, count)
    array_copy(arr, work, count)
    comps = selection_sort(work, count)
    print("Selection on reversed: " + str(comps) + " comparisons")
    print_array(work, count)


def test_duplicates() -> None:
    print("====== Duplicates ======")
    arr: list = [5, 3, 5, 1, 3, 1, 5, 3, 1, 5]
    count: int = 10
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    test_sort_algorithm("Bubble (dupes)", arr, work, count, 1)
    test_sort_algorithm("Merge (dupes)", arr, work, count, 4)
    test_sort_algorithm("Quick (dupes)", arr, work, count, 5)


def test_single_element() -> None:
    print("====== Single Element ======")
    arr: list = [42, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    count: int = 1
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    array_copy(arr, work, count)
    comps: int = bubble_sort(work, count)
    print("Bubble single: " + str(comps) + " comps, val=" + str(work[0]))
    array_copy(arr, work, count)
    comps = merge_sort(work, count)
    print("Merge single: " + str(comps) + " comps, val=" + str(work[0]))


def test_two_elements() -> None:
    print("====== Two Elements ======")
    arr1: list = [10, 5, 0, 0, 0, 0, 0, 0, 0, 0]
    arr2: list = [5, 10, 0, 0, 0, 0, 0, 0, 0, 0]
    count: int = 2
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    array_copy(arr1, work, count)
    comps1: int = bubble_sort(work, count)
    print("Unsorted pair: " + str(comps1) + " comps -> " + str(work[0]) + " " + str(work[1]))
    array_copy(arr2, work, count)
    comps2: int = bubble_sort(work, count)
    print("Sorted pair: " + str(comps2) + " comps -> " + str(work[0]) + " " + str(work[1]))


def test_generated() -> None:
    print("====== Generated Data ======")
    arr: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    generate_test_array(42, 15, arr)
    count: int = 15
    print("Generated: ")
    print_array(arr, count)
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    test_sort_algorithm("Bubble (gen)", arr, work, count, 1)
    test_sort_algorithm("Quick (gen)", arr, work, count, 5)
    test_sort_algorithm("Merge (gen)", arr, work, count, 4)
    test_sort_algorithm("Shell (gen)", arr, work, count, 6)


def test_stability_check() -> None:
    print("====== Stability Check ======")
    arr: list = [30, 10, 20, 10, 30, 20, 10, 0, 0, 0]
    count: int = 7
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    array_copy(arr, work, count)
    comps: int = merge_sort(work, count)
    print("Merge sort result: ")
    print_array(work, count)
    print("Comparisons: " + str(comps))


def main() -> int:
    data1: list = [64, 34, 25, 12, 22, 11, 90, 45, 0, 0]
    test_with_dataset("Dataset 1 (8 elements)", data1, 8)

    data2: list = [5, 1, 4, 2, 8, 3, 7, 6, 9, 10]
    test_with_dataset("Dataset 2 (10 elements)", data2, 10)

    test_counting_sort()
    test_reverse()
    test_already_sorted()
    test_reverse_sorted()
    test_duplicates()
    test_single_element()
    test_two_elements()
    test_generated()
    test_stability_check()

    return 0


if __name__ == "__main__":
    main()
