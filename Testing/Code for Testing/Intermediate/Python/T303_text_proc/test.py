from dataclasses import dataclass, field
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def strlen(s): return len(s)

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def reverse_string(s):
    result = ''
    i = (strlen(s) - 1)
    while (i >= 0):
        ch = str_char_at(s, i)
        result = (result + ch)
        i = (i - 1)
    return result

def is_palindrome(s):
    return (s == reverse_string(s))

def count_char(s, target):
    count = 0
    i = 0
    slen = strlen(s)
    while (i < slen):
        ch = str_char_at(s, i)
        if (ch == target):
            count = (count + 1)
        i = (i + 1)
    return count

def repeat_string(s, times):
    result = ''
    i = 0
    while (i < times):
        result = (result + s)
        i = (i + 1)
    return result

def main():
    print(reverse_string('hello'))
    print(reverse_string('abcdef'))
    if is_palindrome('racecar'):
        print('racecar is palindrome')
    else:
        print('racecar is not palindrome')
    if is_palindrome('hello'):
        print('hello is palindrome')
    else:
        print('hello is not palindrome')
    print(count_char('mississippi', 's'))
    print(count_char('abcabc', 'a'))
    print(repeat_string('ab', 4))
    print(repeat_string('xyz', 2))
    return 0


if __name__ == "__main__":
    main()
