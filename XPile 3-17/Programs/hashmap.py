# @dr memory = manual

from dataclasses import dataclass, field
import sys
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def strlen(s): return len(s)

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def char_val(ch):
    if (ch == 'a'):
        return 97
    if (ch == 'b'):
        return 98
    if (ch == 'c'):
        return 99
    if (ch == 'd'):
        return 100
    if (ch == 'e'):
        return 101
    if (ch == 'f'):
        return 102
    if (ch == 'g'):
        return 103
    if (ch == 'h'):
        return 104
    if (ch == 'i'):
        return 105
    if (ch == 'j'):
        return 106
    if (ch == 'k'):
        return 107
    if (ch == 'l'):
        return 108
    if (ch == 'm'):
        return 109
    if (ch == 'n'):
        return 110
    if (ch == 'o'):
        return 111
    if (ch == 'p'):
        return 112
    if (ch == 'q'):
        return 113
    if (ch == 'r'):
        return 114
    if (ch == 's'):
        return 115
    if (ch == 't'):
        return 116
    if (ch == 'u'):
        return 117
    if (ch == 'v'):
        return 118
    if (ch == 'w'):
        return 119
    if (ch == 'x'):
        return 120
    if (ch == 'y'):
        return 121
    if (ch == 'z'):
        return 122
    if (ch == 'A'):
        return 65
    if (ch == 'B'):
        return 66
    if (ch == 'C'):
        return 67
    if (ch == 'D'):
        return 68
    if (ch == 'E'):
        return 69
    if (ch == 'F'):
        return 70
    if (ch == 'G'):
        return 71
    if (ch == 'H'):
        return 72
    if (ch == 'I'):
        return 73
    if (ch == 'J'):
        return 74
    if (ch == 'K'):
        return 75
    if (ch == 'L'):
        return 76
    if (ch == 'M'):
        return 77
    if (ch == 'N'):
        return 78
    if (ch == 'O'):
        return 79
    if (ch == 'P'):
        return 80
    if (ch == 'Q'):
        return 81
    if (ch == 'R'):
        return 82
    if (ch == 'S'):
        return 83
    if (ch == 'T'):
        return 84
    if (ch == 'U'):
        return 85
    if (ch == 'V'):
        return 86
    if (ch == 'W'):
        return 87
    if (ch == 'X'):
        return 88
    if (ch == 'Y'):
        return 89
    if (ch == 'Z'):
        return 90
    if (ch == '0'):
        return 48
    if (ch == '1'):
        return 49
    if (ch == '2'):
        return 50
    if (ch == '3'):
        return 51
    if (ch == '4'):
        return 52
    if (ch == '5'):
        return 53
    if (ch == '6'):
        return 54
    if (ch == '7'):
        return 55
    if (ch == '8'):
        return 56
    if (ch == '9'):
        return 57
    if (ch == ' '):
        return 32
    if (ch == '_'):
        return 95
    if (ch == '-'):
        return 45
    if (ch == '.'):
        return 46
    return 0

def hash_key(key):
    hash = 5381
    len = strlen(key)
    for i in range(len):
        ch = str_char_at(key, i)
        hash = ((hash * 31) + char_val(ch))
    if (hash < 0):
        hash = (0 - hash)
    return hash

def hm_set(keys, vals, hm_state, key, val):
    cap = hm_state[0]
    if (hm_state[1] >= (cap // 2)):
        new_cap = (cap * 2)
        old_keys = ['', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '']
        old_vals = ['', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '']
        for i in range(cap):
            old_keys[i] = keys[i]
            old_vals[i] = vals[i]
            keys[i] = ''
            vals[i] = ''
        hm_state[0] = new_cap
        hm_state[1] = 0
        for i in range(cap):
            if (old_keys[i] != ''):
                if (old_keys[i] != '__deleted__'):
                    hm_set(keys, vals, hm_state, old_keys[i], old_vals[i])
        cap = new_cap
    hash = hash_key(key)
    index = (hash % cap)
    while (keys[index] != ''):
        if (keys[index] == key):
            vals[index] = val
            return
        if (keys[index] == '__deleted__'):
            break
        index = (index + 1)
        if (index >= cap):
            index = 0
    keys[index] = key
    vals[index] = val
    hm_state[1] = (hm_state[1] + 1)

def hm_get(keys, vals, hm_state, key):
    cap = hm_state[0]
    hash = hash_key(key)
    index = (hash % cap)
    while (keys[index] != ''):
        if (keys[index] == key):
            return (True, vals[index])
        index = (index + 1)
        if (index >= cap):
            index = 0
    return (False, 'key not found')

def hm_has(keys, hm_state, key):
    cap = hm_state[0]
    hash = hash_key(key)
    index = (hash % cap)
    while (keys[index] != ''):
        if (keys[index] == key):
            return True
        index = (index + 1)
        if (index >= cap):
            index = 0
    return False

def hm_delete(keys, vals, hm_state, key):
    cap = hm_state[0]
    hash = hash_key(key)
    index = (hash % cap)
    while (keys[index] != ''):
        if (keys[index] == key):
            keys[index] = '__deleted__'
            vals[index] = ''
            hm_state[1] = (hm_state[1] - 1)
            return
        index = (index + 1)
        if (index >= cap):
            index = 0

def hm_len(hm_state):
    return hm_state[1]

def main():
    keys = ['', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '']
    vals = ['', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '']
    hm_state = [16, 0]
    print('Setting 10 entries...')
    hm_set(keys, vals, hm_state, 'apple', 'red')
    hm_set(keys, vals, hm_state, 'banana', 'yellow')
    hm_set(keys, vals, hm_state, 'cherry', 'red')
    hm_set(keys, vals, hm_state, 'date', 'brown')
    hm_set(keys, vals, hm_state, 'elderberry', 'purple')
    hm_set(keys, vals, hm_state, 'fig', 'green')
    hm_set(keys, vals, hm_state, 'grape', 'purple')
    hm_set(keys, vals, hm_state, 'honeydew', 'green')
    hm_set(keys, vals, hm_state, 'kiwi', 'brown')
    hm_set(keys, vals, hm_state, 'lemon', 'yellow')
    print('Length:')
    print(hm_len(hm_state))
    print('')
    print('Get tests:')
    _r0 = hm_get(keys, vals, hm_state, 'apple')
    if not _r0[0]:
        print("error: " + str(_r0[1]), file=sys.stderr)
        sys.exit(1)
    v1 = _r0[1]
    print('apple -> {v1}')
    _r1 = hm_get(keys, vals, hm_state, 'banana')
    if not _r1[0]:
        print("error: " + str(_r1[1]), file=sys.stderr)
        sys.exit(1)
    v2 = _r1[1]
    print('banana -> {v2}')
    _r2 = hm_get(keys, vals, hm_state, 'grape')
    if not _r2[0]:
        print("error: " + str(_r2[1]), file=sys.stderr)
        sys.exit(1)
    v3 = _r2[1]
    print('grape -> {v3}')
    _r3 = hm_get(keys, vals, hm_state, 'kiwi')
    if not _r3[0]:
        print("error: " + str(_r3[1]), file=sys.stderr)
        sys.exit(1)
    v4 = _r3[1]
    print('kiwi -> {v4}')
    _r4 = hm_get(keys, vals, hm_state, 'lemon')
    if not _r4[0]:
        print("error: " + str(_r4[1]), file=sys.stderr)
        sys.exit(1)
    v5 = _r4[1]
    print('lemon -> {v5}')
    print('')
    print('Has tests:')
    print('has apple:')
    print(hm_has(keys, hm_state, 'apple'))
    print('has mango:')
    print(hm_has(keys, hm_state, 'mango'))
    print('has cherry:')
    print(hm_has(keys, hm_state, 'cherry'))
    print('has zzz:')
    print(hm_has(keys, hm_state, 'zzz'))
    print('has fig:')
    print(hm_has(keys, hm_state, 'fig'))
    print('')
    print('Updating 5 entries...')
    hm_set(keys, vals, hm_state, 'apple', 'green')
    hm_set(keys, vals, hm_state, 'banana', 'brown')
    hm_set(keys, vals, hm_state, 'cherry', 'dark red')
    hm_set(keys, vals, hm_state, 'grape', 'green')
    hm_set(keys, vals, hm_state, 'lemon', 'bright yellow')
    print('Length after updates (should be same):')
    print(hm_len(hm_state))
    _r5 = hm_get(keys, vals, hm_state, 'apple')
    if not _r5[0]:
        print("error: " + str(_r5[1]), file=sys.stderr)
        sys.exit(1)
    v6 = _r5[1]
    print('apple -> {v6}')
    _r6 = hm_get(keys, vals, hm_state, 'cherry')
    if not _r6[0]:
        print("error: " + str(_r6[1]), file=sys.stderr)
        sys.exit(1)
    v7 = _r6[1]
    print('cherry -> {v7}')
    print('')
    print('Deleting fig, date, elderberry')
    hm_delete(keys, vals, hm_state, 'fig')
    hm_delete(keys, vals, hm_state, 'date')
    hm_delete(keys, vals, hm_state, 'elderberry')
    print('Length after deletes:')
    print(hm_len(hm_state))
    print('has fig:')
    print(hm_has(keys, hm_state, 'fig'))
    print('has date:')
    print(hm_has(keys, hm_state, 'date'))
    print('')
    print('Adding 10 more entries...')
    hm_set(keys, vals, hm_state, 'mango', 'orange')
    hm_set(keys, vals, hm_state, 'nectarine', 'orange')
    hm_set(keys, vals, hm_state, 'orange', 'orange')
    hm_set(keys, vals, hm_state, 'papaya', 'orange')
    hm_set(keys, vals, hm_state, 'quince', 'yellow')
    hm_set(keys, vals, hm_state, 'raspberry', 'red')
    hm_set(keys, vals, hm_state, 'strawberry', 'red')
    hm_set(keys, vals, hm_state, 'tangerine', 'orange')
    hm_set(keys, vals, hm_state, 'watermelon', 'green')
    hm_set(keys, vals, hm_state, 'plum', 'purple')
    print('Length:')
    print(hm_len(hm_state))
    print('')
    print('Verify after expansion:')
    _r7 = hm_get(keys, vals, hm_state, 'apple')
    if not _r7[0]:
        print("error: " + str(_r7[1]), file=sys.stderr)
        sys.exit(1)
    va = _r7[1]
    print('apple -> {va}')
    _r8 = hm_get(keys, vals, hm_state, 'banana')
    if not _r8[0]:
        print("error: " + str(_r8[1]), file=sys.stderr)
        sys.exit(1)
    vb = _r8[1]
    print('banana -> {vb}')
    _r9 = hm_get(keys, vals, hm_state, 'mango')
    if not _r9[0]:
        print("error: " + str(_r9[1]), file=sys.stderr)
        sys.exit(1)
    vm = _r9[1]
    print('mango -> {vm}')
    _r10 = hm_get(keys, vals, hm_state, 'orange')
    if not _r10[0]:
        print("error: " + str(_r10[1]), file=sys.stderr)
        sys.exit(1)
    vo = _r10[1]
    print('orange -> {vo}')
    _r11 = hm_get(keys, vals, hm_state, 'raspberry')
    if not _r11[0]:
        print("error: " + str(_r11[1]), file=sys.stderr)
        sys.exit(1)
    vr = _r11[1]
    print('raspberry -> {vr}')
    _r12 = hm_get(keys, vals, hm_state, 'watermelon')
    if not _r12[0]:
        print("error: " + str(_r12[1]), file=sys.stderr)
        sys.exit(1)
    vw = _r12[1]
    print('watermelon -> {vw}')
    _r13 = hm_get(keys, vals, hm_state, 'plum')
    if not _r13[0]:
        print("error: " + str(_r13[1]), file=sys.stderr)
        sys.exit(1)
    vp = _r13[1]
    print('plum -> {vp}')
    print('has strawberry:')
    print(hm_has(keys, hm_state, 'strawberry'))
    print('has fig (deleted):')
    print(hm_has(keys, hm_state, 'fig'))
    print('Final length:')
    print(hm_len(hm_state))
    return 0


if __name__ == "__main__":
    main()
