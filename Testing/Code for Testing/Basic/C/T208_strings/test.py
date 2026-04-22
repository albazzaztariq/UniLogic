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


def main():
    hello = (('Hello' + ' ') + 'World')
    print(hello)
    len = strlen(hello)
    print(len)
    first = str_char_at(hello, 0)
    print(first)
    sub = str_substr(hello, 0, 5)
    print(sub)
    greeting = 'Good Morning'
    part = greeting[0:4]
    print(part)
    a = 'abc'
    b = 'abc'
    if (a == b):
        print('equal')
    c = 'xyz'
    if (a != c):
        print('not equal')
    sentence = 'the quick brown fox'
    if ('quick' in sentence):
        print('found quick')
    result = ''
    i = 0
    while (i < 3):
        result = (result + str(i))
        i = (i + 1)
    print(result)
    return 0


if __name__ == "__main__":
    main()
