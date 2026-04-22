from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def reverse_string(s):
    result = ''
    i = (len(s) - 1)
    while (i >= 0):
        result = (result + s[i])
        i = (i - 1)
    return result

def is_palindrome(s):
    return (s == reverse_string(s))

def count_char(s, ch):
    count = 0
    i = 0
    while (i < len(s)):
        if (s[i] == ch):
            count = (count + 1)
        i = (i + 1)
    return count

def to_upper(s):
    result = ''
    i = 0
    while (i < len(s)):
        c = ord(s[i])
        if ((c >= 97) and (c <= 122)):
            result = (result + chr((c - 32)))
        else:
            result = (result + s[i])
        i = (i + 1)
    return result

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
    print(to_upper('hello world'))
    print(to_upper('Python 3.12'))
    print(repeat_string('ab', 4))
    print(repeat_string('xyz', 2))
    return 0


if __name__ == "__main__":
    main()
