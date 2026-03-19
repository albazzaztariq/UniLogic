from dataclasses import dataclass, field
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def strlen(s): return len(s)

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def wc(text):
    len = strlen(text)
    lines = 0
    words = 0
    chars = len
    in_word = False
    for i in range(len):
        ch = str_char_at(text, i)
        is_space = False
        if (ch == ' '):
            is_space = True
        if (ch == '\t'):
            is_space = True
        if (ch == 'r'):
            is_space = True
        if (ch == '\n'):
            is_space = True
            lines = (lines + 1)
        if (is_space == False):
            if (in_word == False):
                words = (words + 1)
            in_word = True
        else:
            in_word = False
    print('  {lines} {words} {chars}')

def main():
    print('Test 1: empty string')
    wc('')
    print('Test 2: single word')
    wc('hello')
    print('Test 3: simple sentence')
    wc('hello world')
    print('Test 4: two lines')
    wc('The quick brown fox\njumps over the lazy dog\n')
    print('Test 5: paragraphs with blank line')
    wc('First line\nSecond line\n\nFourth line after blank\n')
    print('Test 6: whitespace only')
    wc('   \n\n   \n')
    return 0


if __name__ == "__main__":
    main()
