from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def filter_positive(data, count, out):
    out_count = 0
    i = 0
    while (i < count):
        if (data[i] > 0):
            out[out_count] = data[i]
            out_count = (out_count + 1)
        i = (i + 1)
    return out_count

def filter_above(data, count, out, threshold):
    out_count = 0
    i = 0
    while (i < count):
        if (data[i] > threshold):
            out[out_count] = data[i]
            out_count = (out_count + 1)
        i = (i + 1)
    return out_count

def filter_below(data, count, out, threshold):
    out_count = 0
    i = 0
    while (i < count):
        if (data[i] < threshold):
            out[out_count] = data[i]
            out_count = (out_count + 1)
        i = (i + 1)
    return out_count

def transform_double(data, count):
    i = 0
    while (i < count):
        data[i] = (data[i] * 2)
        i = (i + 1)

def transform_add(data, count, value):
    i = 0
    while (i < count):
        data[i] = (data[i] + value)
        i = (i + 1)

def transform_clamp(data, count, lo, hi):
    i = 0
    while (i < count):
        if (data[i] < lo):
            data[i] = lo
        if (data[i] > hi):
            data[i] = hi
        i = (i + 1)

def agg_sum(data, count):
    total = 0
    i = 0
    while (i < count):
        total = (total + data[i])
        i = (i + 1)
    return total

def agg_min(data, count):
    result = data[0]
    i = 1
    while (i < count):
        if (data[i] < result):
            result = data[i]
        i = (i + 1)
    return result

def agg_max(data, count):
    result = data[0]
    i = 1
    while (i < count):
        if (data[i] > result):
            result = data[i]
        i = (i + 1)
    return result

def agg_count_eq(data, count, target):
    result = 0
    i = 0
    while (i < count):
        if (data[i] == target):
            result = (result + 1)
        i = (i + 1)
    return result

def print_array(data, count):
    result = ''
    i = 0
    while (i < count):
        if (i > 0):
            result = (result + ' ')
        result = (result + str(data[i]))
        i = (i + 1)
    print(result)

def copy_array(src, dst, count):
    i = 0
    while (i < count):
        dst[i] = src[i]
        i = (i + 1)

def main():
    raw = [(-5), 12, 0, 8, (-3), 15, 7, (-1), 20, 3, 0, 11, (-8), 6, 25]
    raw_count = 15
    buf = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    print('--- Stage 1: Filter positive ---')
    stage1_count = filter_positive(raw, raw_count, buf)
    print(('count: ' + str(stage1_count)))
    print_array(buf, stage1_count)
    print(('sum: ' + str(agg_sum(buf, stage1_count))))
    print('--- Stage 2: Transform double ---')
    transform_double(buf, stage1_count)
    print_array(buf, stage1_count)
    print(('sum: ' + str(agg_sum(buf, stage1_count))))
    print(('min: ' + str(agg_min(buf, stage1_count))))
    print(('max: ' + str(agg_max(buf, stage1_count))))
    print('--- Stage 3: Filter above 20 ---')
    buf2 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    stage3_count = filter_above(buf, stage1_count, buf2, 20)
    print(('count: ' + str(stage3_count)))
    print_array(buf2, stage3_count)
    print('--- Stage 4: Clamp to [25, 40] ---')
    transform_clamp(buf2, stage3_count, 25, 40)
    print_array(buf2, stage3_count)
    print(('sum: ' + str(agg_sum(buf2, stage3_count))))
    print('--- Pipeline 2: filter below 10, add 100 ---')
    buf3 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    p2_count = filter_below(raw, raw_count, buf3, 10)
    print(('filtered: ' + str(p2_count)))
    transform_add(buf3, p2_count, 100)
    print_array(buf3, p2_count)
    print(('sum: ' + str(agg_sum(buf3, p2_count))))
    print('--- Aggregation ---')
    print(('count 0 in raw: ' + str(agg_count_eq(raw, raw_count, 0))))
    print(('count 12 in raw: ' + str(agg_count_eq(raw, raw_count, 12))))
    return 0


if __name__ == "__main__":
    main()
