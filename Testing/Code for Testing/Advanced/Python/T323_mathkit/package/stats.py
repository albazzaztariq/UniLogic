"""Statistical functions on integer arrays."""


def stat_sum(data: list, count: int) -> int:
    total: int = 0
    i: int = 0
    while i < count:
        total = total + data[i]
        i = i + 1
    return total


def stat_min(data: list, count: int) -> int:
    result: int = data[0]
    i: int = 1
    while i < count:
        if data[i] < result:
            result = data[i]
        i = i + 1
    return result


def stat_max(data: list, count: int) -> int:
    result: int = data[0]
    i: int = 1
    while i < count:
        if data[i] > result:
            result = data[i]
        i = i + 1
    return result


def stat_mean_x10(data: list, count: int) -> int:
    """Return mean * 10 to avoid float division."""
    total: int = stat_sum(data, count)
    return (total * 10) // count


def stat_variance_x100(data: list, count: int) -> int:
    """Return variance * 100 using integer arithmetic."""
    mean_x10: int = stat_mean_x10(data, count)
    total: int = 0
    i: int = 0
    while i < count:
        diff: int = data[i] * 10 - mean_x10
        total = total + diff * diff
        i = i + 1
    return total // count


def stat_range(data: list, count: int) -> int:
    return stat_max(data, count) - stat_min(data, count)


def stat_median(data: list, count: int, work: list) -> int:
    """Return median. Requires work array of same size. Sorts work array."""
    i: int = 0
    while i < count:
        work[i] = data[i]
        i = i + 1
    # Simple sort for median
    i = 0
    while i < count - 1:
        j: int = i + 1
        while j < count:
            if work[j] < work[i]:
                temp: int = work[i]
                work[i] = work[j]
                work[j] = temp
            j = j + 1
        i = i + 1
    if count % 2 == 1:
        return work[count // 2]
    return (work[count // 2 - 1] + work[count // 2]) // 2
