from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def bubble_sort(arr, count):
    comparisons = 0
    i = 0
    while (i < (count - 1)):
        j = 0
        while (j < ((count - 1) - i)):
            comparisons = (comparisons + 1)
            if (arr[j] > arr[(j + 1)]):
                temp = arr[j]
                arr[j] = arr[(j + 1)]
                arr[(j + 1)] = temp
            j = (j + 1)
        i = (i + 1)
    return comparisons

def selection_sort(arr, count):
    comparisons = 0
    i = 0
    while (i < (count - 1)):
        min_idx = i
        j = (i + 1)
        while (j < count):
            comparisons = (comparisons + 1)
            if (arr[j] < arr[min_idx]):
                min_idx = j
            j = (j + 1)
        if (min_idx != i):
            temp = arr[i]
            arr[i] = arr[min_idx]
            arr[min_idx] = temp
        i = (i + 1)
    return comparisons

def insertion_sort(arr, count):
    comparisons = 0
    i = 1
    while (i < count):
        key = arr[i]
        j = (i - 1)
        done = False
        while ((j >= 0) and (not done)):
            comparisons = (comparisons + 1)
            if (arr[j] > key):
                arr[(j + 1)] = arr[j]
                j = (j - 1)
            else:
                done = True
        arr[(j + 1)] = key
        i = (i + 1)
    return comparisons

def array_copy(src, dst, count):
    i = 0
    while (i < count):
        dst[i] = src[i]
        i = (i + 1)

def is_sorted(arr, count):
    i = 0
    while (i < (count - 1)):
        if (arr[i] > arr[(i + 1)]):
            return False
        i = (i + 1)
    return True

def print_array(arr, count):
    result = ''
    i = 0
    while (i < count):
        if (i > 0):
            result = (result + ' ')
        result = (result + str(arr[i]))
        i = (i + 1)
    print(result)

def array_sum(arr, count):
    total = 0
    i = 0
    while (i < count):
        total = (total + arr[i])
        i = (i + 1)
    return total

def test_sort(name, original, work, count, sort_type):
    array_copy(original, work, count)
    comparisons = 0
    if (sort_type == 1):
        comparisons = bubble_sort(work, count)
    if (sort_type == 2):
        comparisons = selection_sort(work, count)
    if (sort_type == 3):
        comparisons = insertion_sort(work, count)
    print(((((((name + ': comps=') + str(comparisons)) + ' sorted=') + str(is_sorted(work, count))) + ' sum=') + str(array_sum(work, count))))

def main():
    data = [64, 34, 25, 12, 22, 11, 90, 45, 0, 0]
    count = 8
    work = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    print('--- Dataset 1 ---')
    print_array(data, count)
    test_sort('Bubble', data, work, count, 1)
    test_sort('Selection', data, work, count, 2)
    test_sort('Insertion', data, work, count, 3)
    data2 = [5, 1, 4, 2, 8, 3, 7, 6, 9, 10]
    count2 = 10
    print('--- Dataset 2 ---')
    print_array(data2, count2)
    test_sort('Bubble', data2, work, count2, 1)
    test_sort('Selection', data2, work, count2, 2)
    test_sort('Insertion', data2, work, count2, 3)
    sorted_data = [1, 2, 3, 4, 5, 6, 7, 8, 0, 0]
    count3 = 8
    print('--- Already sorted ---')
    test_sort('Bubble', sorted_data, work, count3, 1)
    test_sort('Insertion', sorted_data, work, count3, 3)
    rev_data = [8, 7, 6, 5, 4, 3, 2, 1, 0, 0]
    print('--- Reverse sorted ---')
    test_sort('Bubble', rev_data, work, count3, 1)
    test_sort('Insertion', rev_data, work, count3, 3)
    single = [42, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    print('--- Single ---')
    test_sort('Bubble', single, work, 1, 1)
    return 0


if __name__ == "__main__":
    main()
