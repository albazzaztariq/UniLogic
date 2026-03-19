from dataclasses import dataclass, field
import sys

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def bisect_arr(arr, lo, hi, val):
    while (lo < hi):
        mid = ((lo + hi) // 2)
        if (arr[mid] < val):
            lo = (mid + 1)
        else:
            hi = mid
    return lo

def bisect_arr_r(arr, lo, hi, val):
    while (lo < hi):
        mid = ((lo + hi) // 2)
        if (arr[mid] <= val):
            lo = (mid + 1)
        else:
            hi = mid
    return lo

def shift_up(data, pos, end_idx):
    i = end_idx
    while (i > pos):
        data[i] = data[(i - 1)]
        i = (i - 1)

def shift_down(data, pos, end_idx):
    i = pos
    while (i < (end_idx - 1)):
        data[i] = data[(i + 1)]
        i = (i + 1)

def shift_data_up(data, from_pos, total):
    i = total
    while (i > from_pos):
        data[i] = data[(i - 1)]
        i = (i - 1)

def sl_insort(data, start, len, val):
    pos = bisect_arr(data, start, (start + len), val)
    shift_up(data, pos, (start + len))
    data[pos] = val
    return pos

def sl_expand(data, sub_start, sub_len, sub_max, state, pos):
    load = state[2]
    num_subs = state[0]
    if (sub_len[pos] > (load * 2)):
        old_start = sub_start[pos]
        split_at = load
        i = num_subs
        while (i > (pos + 1)):
            sub_start[i] = sub_start[(i - 1)]
            sub_len[i] = sub_len[(i - 1)]
            sub_max[i] = sub_max[(i - 1)]
            i = (i - 1)
        sub_start[(pos + 1)] = (old_start + split_at)
        sub_len[(pos + 1)] = (sub_len[pos] - split_at)
        sub_max[(pos + 1)] = data[((old_start + sub_len[pos]) - 1)]
        sub_len[pos] = split_at
        sub_max[pos] = data[((old_start + split_at) - 1)]
        state[0] = (num_subs + 1)

def sl_add(data, sub_start, sub_len, sub_max, state, val):
    num_subs = state[0]
    total = state[1]
    if (num_subs == 0):
        data[0] = val
        sub_start[0] = 0
        sub_len[0] = 1
        sub_max[0] = val
        state[0] = 1
        state[1] = 1
        return
    pos = 0
    for k in range(num_subs):
        if (sub_max[k] < val):
            pos = (k + 1)
        else:
            break
    if (pos == num_subs):
        pos = (num_subs - 1)
        ins_at = (sub_start[pos] + sub_len[pos])
        i = total
        while (i > ins_at):
            data[i] = data[(i - 1)]
            i = (i - 1)
        data[ins_at] = val
        sub_len[pos] = (sub_len[pos] + 1)
        sub_max[pos] = val
        for s in range((pos + 1), num_subs):
            sub_start[s] = (sub_start[s] + 1)
    else:
        s_start = sub_start[pos]
        s_len = sub_len[pos]
        ins_pos = bisect_arr(data, s_start, (s_start + s_len), val)
        i = total
        while (i > ins_pos):
            data[i] = data[(i - 1)]
            i = (i - 1)
        data[ins_pos] = val
        sub_len[pos] = (sub_len[pos] + 1)
        if (val > sub_max[pos]):
            sub_max[pos] = val
        for s in range((pos + 1), num_subs):
            sub_start[s] = (sub_start[s] + 1)
    state[1] = (total + 1)
    sl_expand(data, sub_start, sub_len, sub_max, state, pos)

def sl_remove(data, sub_start, sub_len, sub_max, state, val):
    num_subs = state[0]
    if (num_subs == 0):
        return (False, 'value not in list')
    pos = 0
    for k in range(num_subs):
        if (sub_max[k] < val):
            pos = (k + 1)
    if (pos == num_subs):
        return (False, 'value not in list')
    s_start = sub_start[pos]
    s_len = sub_len[pos]
    idx = bisect_arr(data, s_start, (s_start + s_len), val)
    if (idx >= (s_start + s_len)):
        return (False, 'value not in list')
    if (data[idx] != val):
        return (False, 'value not in list')
    total = state[1]
    i = idx
    while (i < (total - 1)):
        data[i] = data[(i + 1)]
        i = (i + 1)
    sub_len[pos] = (sub_len[pos] - 1)
    state[1] = (total - 1)
    for s in range((pos + 1), num_subs):
        sub_start[s] = (sub_start[s] - 1)
    if (sub_len[pos] == 0):
        j = pos
        while (j < (num_subs - 1)):
            sub_start[j] = sub_start[(j + 1)]
            sub_len[j] = sub_len[(j + 1)]
            sub_max[j] = sub_max[(j + 1)]
            j = (j + 1)
        state[0] = (num_subs - 1)
    else:
        sub_max[pos] = data[((sub_start[pos] + sub_len[pos]) - 1)]
    return (True, 1)

def sl_contains(data, sub_start, sub_len, sub_max, state, val):
    num_subs = state[0]
    if (num_subs == 0):
        return False
    pos = 0
    for k in range(num_subs):
        if (sub_max[k] < val):
            pos = (k + 1)
    if (pos == num_subs):
        return False
    s_start = sub_start[pos]
    s_len = sub_len[pos]
    idx = bisect_arr(data, s_start, (s_start + s_len), val)
    if (idx >= (s_start + s_len)):
        return False
    return (data[idx] == val)

def sl_get(data, sub_start, sub_len, state, idx):
    remaining = idx
    num_subs = state[0]
    for s in range(num_subs):
        if (remaining < sub_len[s]):
            return data[(sub_start[s] + remaining)]
        remaining = (remaining - sub_len[s])
    return 0

def sl_len(state):
    return state[1]

def sl_print(data, state):
    total = state[1]
    result = '['
    for i in range(total):
        if (i > 0):
            result = (result + ', ')
        result = (result + '{data[i]}')
    result = (result + ']')
    print(result)

def main():
    data = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    sub_start = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    sub_len = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    sub_max = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    state = [0, 0, 8]
    print('Adding: 50, 20, 80, 10, 40, 90, 30, 70, 60, 15')
    sl_add(data, sub_start, sub_len, sub_max, state, 50)
    sl_add(data, sub_start, sub_len, sub_max, state, 20)
    sl_add(data, sub_start, sub_len, sub_max, state, 80)
    sl_add(data, sub_start, sub_len, sub_max, state, 10)
    sl_add(data, sub_start, sub_len, sub_max, state, 40)
    sl_add(data, sub_start, sub_len, sub_max, state, 90)
    sl_add(data, sub_start, sub_len, sub_max, state, 30)
    sl_add(data, sub_start, sub_len, sub_max, state, 70)
    sl_add(data, sub_start, sub_len, sub_max, state, 60)
    sl_add(data, sub_start, sub_len, sub_max, state, 15)
    print('After 10 adds:')
    sl_print(data, state)
    print('Length:')
    print(sl_len(state))
    print('Contains 40:')
    print(sl_contains(data, sub_start, sub_len, sub_max, state, 40))
    print('Contains 99:')
    print(sl_contains(data, sub_start, sub_len, sub_max, state, 99))
    print('Contains 10:')
    print(sl_contains(data, sub_start, sub_len, sub_max, state, 10))
    print('Contains 90:')
    print(sl_contains(data, sub_start, sub_len, sub_max, state, 90))
    print('Element at index 0:')
    print(sl_get(data, sub_start, sub_len, state, 0))
    print('Element at index 5:')
    print(sl_get(data, sub_start, sub_len, state, 5))
    print('Element at index 9:')
    print(sl_get(data, sub_start, sub_len, state, 9))
    print('Removing 20, 80, 15')
    _r0 = sl_remove(data, sub_start, sub_len, sub_max, state, 20)
    if not _r0[0]:
        print("error: " + str(_r0[1]), file=sys.stderr)
        sys.exit(1)
    _r0[1]
    _r1 = sl_remove(data, sub_start, sub_len, sub_max, state, 80)
    if not _r1[0]:
        print("error: " + str(_r1[1]), file=sys.stderr)
        sys.exit(1)
    _r1[1]
    _r2 = sl_remove(data, sub_start, sub_len, sub_max, state, 15)
    if not _r2[0]:
        print("error: " + str(_r2[1]), file=sys.stderr)
        sys.exit(1)
    _r2[1]
    print('After removes:')
    sl_print(data, state)
    print('Length:')
    print(sl_len(state))
    print('Adding: 25, 25, 55, 5, 95, 42')
    sl_add(data, sub_start, sub_len, sub_max, state, 25)
    sl_add(data, sub_start, sub_len, sub_max, state, 25)
    sl_add(data, sub_start, sub_len, sub_max, state, 55)
    sl_add(data, sub_start, sub_len, sub_max, state, 5)
    sl_add(data, sub_start, sub_len, sub_max, state, 95)
    sl_add(data, sub_start, sub_len, sub_max, state, 42)
    print('Final state:')
    sl_print(data, state)
    print('Length:')
    print(sl_len(state))
    print('Elements by index:')
    total = sl_len(state)
    for i in range(total):
        print(sl_get(data, sub_start, sub_len, state, i))
    return 0


if __name__ == "__main__":
    main()
