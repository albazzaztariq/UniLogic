from dataclasses import dataclass, field
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def str_substr(s, start, length): return s[start:start+length]
def strlen(s): return len(s)

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def count_char(s, target):
    len = strlen(s)
    count = 0
    i = 0
    while (i < len):
        ch = str_char_at(s, i)
        if (ch == target):
            count = (count + 1)
        i = (i + 1)
    return count

def first_word(s):
    len = strlen(s)
    i = 0
    while (i < len):
        ch = str_char_at(s, i)
        if (ch == ' '):
            return str_substr(s, 0, i)
        i = (i + 1)
    return s

def main():
    msg = 'hello world'
    print(strlen(msg))
    ch0 = str_char_at(msg, 0)
    print(ch0)
    ch6 = str_char_at(msg, 6)
    print(ch6)
    sub = str_substr(msg, 0, 5)
    print(sub)
    print(count_char(msg, 'l'))
    print(count_char(msg, 'o'))
    print(first_word('foo bar baz'))
    print(first_word('single'))
    data = [10, 20, 30, 40, 50]
    total = 0
    for val in data:
        total = (total + val)
    print(total)
    nums = [9, 1, 5, 3, 7]
    nums.sort()
    print(nums[0])
    print(nums[4])
    return 0


if __name__ == "__main__":
    main()
