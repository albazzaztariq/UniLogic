"""String processing toolkit — string manipulation, pattern matching,
encoding, formatting, and text analysis using only basic operations."""


def str_len(s: str) -> int:
    return len(s)


def str_char_at(s: str, idx: int) -> str:
    return s[idx:idx + 1]


def str_reverse(s: str) -> str:
    result: str = ""
    i: int = len(s) - 1
    while i >= 0:
        result = result + s[i:i + 1]
        i = i - 1
    return result


def str_to_upper_char(ch: str) -> str:
    if ch == "a":
        return "A"
    if ch == "b":
        return "B"
    if ch == "c":
        return "C"
    if ch == "d":
        return "D"
    if ch == "e":
        return "E"
    if ch == "f":
        return "F"
    if ch == "g":
        return "G"
    if ch == "h":
        return "H"
    if ch == "i":
        return "I"
    if ch == "j":
        return "J"
    if ch == "k":
        return "K"
    if ch == "l":
        return "L"
    if ch == "m":
        return "M"
    if ch == "n":
        return "N"
    if ch == "o":
        return "O"
    if ch == "p":
        return "P"
    if ch == "q":
        return "Q"
    if ch == "r":
        return "R"
    if ch == "s":
        return "S"
    if ch == "t":
        return "T"
    if ch == "u":
        return "U"
    if ch == "v":
        return "V"
    if ch == "w":
        return "W"
    if ch == "x":
        return "X"
    if ch == "y":
        return "Y"
    if ch == "z":
        return "Z"
    return ch


def str_to_lower_char(ch: str) -> str:
    if ch == "A":
        return "a"
    if ch == "B":
        return "b"
    if ch == "C":
        return "c"
    if ch == "D":
        return "d"
    if ch == "E":
        return "e"
    if ch == "F":
        return "f"
    if ch == "G":
        return "g"
    if ch == "H":
        return "h"
    if ch == "I":
        return "i"
    if ch == "J":
        return "j"
    if ch == "K":
        return "k"
    if ch == "L":
        return "l"
    if ch == "M":
        return "m"
    if ch == "N":
        return "n"
    if ch == "O":
        return "o"
    if ch == "P":
        return "p"
    if ch == "Q":
        return "q"
    if ch == "R":
        return "r"
    if ch == "S":
        return "s"
    if ch == "T":
        return "t"
    if ch == "U":
        return "u"
    if ch == "V":
        return "v"
    if ch == "W":
        return "w"
    if ch == "X":
        return "x"
    if ch == "Y":
        return "y"
    if ch == "Z":
        return "z"
    return ch


def str_upper(s: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        result = result + str_to_upper_char(s[i:i + 1])
        i = i + 1
    return result


def str_lower(s: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        result = result + str_to_lower_char(s[i:i + 1])
        i = i + 1
    return result


def is_alpha(ch: str) -> bool:
    upper: str = str_to_upper_char(ch)
    if upper != ch:
        return True
    lower: str = str_to_lower_char(ch)
    if lower != ch:
        return True
    return False


def is_digit_char(ch: str) -> bool:
    if ch == "0":
        return True
    if ch == "1":
        return True
    if ch == "2":
        return True
    if ch == "3":
        return True
    if ch == "4":
        return True
    if ch == "5":
        return True
    if ch == "6":
        return True
    if ch == "7":
        return True
    if ch == "8":
        return True
    if ch == "9":
        return True
    return False


def is_vowel(ch: str) -> bool:
    lower: str = str_to_lower_char(ch)
    if lower == "a":
        return True
    if lower == "e":
        return True
    if lower == "i":
        return True
    if lower == "o":
        return True
    if lower == "u":
        return True
    return False


def count_vowels(s: str) -> int:
    count: int = 0
    i: int = 0
    while i < len(s):
        if is_vowel(s[i:i + 1]):
            count = count + 1
        i = i + 1
    return count


def count_consonants(s: str) -> int:
    count: int = 0
    i: int = 0
    while i < len(s):
        ch: str = s[i:i + 1]
        if is_alpha(ch) and not is_vowel(ch):
            count = count + 1
        i = i + 1
    return count


def count_digits(s: str) -> int:
    count: int = 0
    i: int = 0
    while i < len(s):
        if is_digit_char(s[i:i + 1]):
            count = count + 1
        i = i + 1
    return count


def count_spaces(s: str) -> int:
    count: int = 0
    i: int = 0
    while i < len(s):
        if s[i:i + 1] == " ":
            count = count + 1
        i = i + 1
    return count


def is_palindrome(s: str) -> bool:
    left: int = 0
    right: int = len(s) - 1
    while left < right:
        if s[left:left + 1] != s[right:right + 1]:
            return False
        left = left + 1
        right = right - 1
    return True


def str_repeat(s: str, times: int) -> str:
    result: str = ""
    i: int = 0
    while i < times:
        result = result + s
        i = i + 1
    return result


def str_pad_left(s: str, width: int, pad_char: str) -> str:
    result: str = s
    while len(result) < width:
        result = pad_char + result
    return result


def str_pad_right(s: str, width: int, pad_char: str) -> str:
    result: str = s
    while len(result) < width:
        result = result + pad_char
    return result


def str_contains(haystack: str, needle: str) -> bool:
    h_len: int = len(haystack)
    n_len: int = len(needle)
    if n_len == 0:
        return True
    if n_len > h_len:
        return False
    i: int = 0
    while i <= h_len - n_len:
        found: bool = True
        j: int = 0
        while j < n_len:
            if haystack[i + j:i + j + 1] != needle[j:j + 1]:
                found = False
            j = j + 1
        if found:
            return True
        i = i + 1
    return False


def str_index_of(haystack: str, needle: str) -> int:
    h_len: int = len(haystack)
    n_len: int = len(needle)
    if n_len == 0:
        return 0
    if n_len > h_len:
        return -1
    i: int = 0
    while i <= h_len - n_len:
        found: bool = True
        j: int = 0
        while j < n_len:
            if haystack[i + j:i + j + 1] != needle[j:j + 1]:
                found = False
            j = j + 1
        if found:
            return i
        i = i + 1
    return -1


def str_count_occurrences(haystack: str, needle: str) -> int:
    count: int = 0
    h_len: int = len(haystack)
    n_len: int = len(needle)
    if n_len == 0:
        return 0
    i: int = 0
    while i <= h_len - n_len:
        found: bool = True
        j: int = 0
        while j < n_len:
            if haystack[i + j:i + j + 1] != needle[j:j + 1]:
                found = False
            j = j + 1
        if found:
            count = count + 1
            i = i + n_len
        else:
            i = i + 1
    return count


def str_replace_char(s: str, old_ch: str, new_ch: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        ch: str = s[i:i + 1]
        if ch == old_ch:
            result = result + new_ch
        else:
            result = result + ch
        i = i + 1
    return result


def str_trim_left(s: str) -> str:
    i: int = 0
    while i < len(s) and s[i:i + 1] == " ":
        i = i + 1
    return s[i:len(s)]


def str_trim_right(s: str) -> str:
    i: int = len(s) - 1
    while i >= 0 and s[i:i + 1] == " ":
        i = i - 1
    return s[0:i + 1]


def str_trim(s: str) -> str:
    return str_trim_left(str_trim_right(s))


def str_starts_with(s: str, prefix: str) -> bool:
    p_len: int = len(prefix)
    if p_len > len(s):
        return False
    i: int = 0
    while i < p_len:
        if s[i:i + 1] != prefix[i:i + 1]:
            return False
        i = i + 1
    return True


def str_ends_with(s: str, suffix: str) -> bool:
    s_len: int = len(s)
    x_len: int = len(suffix)
    if x_len > s_len:
        return False
    offset: int = s_len - x_len
    i: int = 0
    while i < x_len:
        if s[offset + i:offset + i + 1] != suffix[i:i + 1]:
            return False
        i = i + 1
    return True


def caesar_encrypt(s: str, shift: int) -> str:
    alphabet: str = "abcdefghijklmnopqrstuvwxyz"
    result: str = ""
    i: int = 0
    while i < len(s):
        ch: str = s[i:i + 1]
        lower_ch: str = str_to_lower_char(ch)
        pos: int = str_index_of(alphabet, lower_ch)
        if pos >= 0:
            new_pos: int = (pos + shift) % 26
            new_ch: str = alphabet[new_pos:new_pos + 1]
            if ch != lower_ch:
                new_ch = str_to_upper_char(new_ch)
            result = result + new_ch
        else:
            result = result + ch
        i = i + 1
    return result


def caesar_decrypt(s: str, shift: int) -> str:
    return caesar_encrypt(s, 26 - shift)


def run_length_encode(s: str) -> str:
    if len(s) == 0:
        return ""
    result: str = ""
    i: int = 0
    while i < len(s):
        ch: str = s[i:i + 1]
        count: int = 1
        while i + count < len(s) and s[i + count:i + count + 1] == ch:
            count = count + 1
        result = result + str(count) + ch
        i = i + count
    return result


def word_count(s: str) -> int:
    count: int = 0
    in_word: bool = False
    i: int = 0
    while i < len(s):
        ch: str = s[i:i + 1]
        if ch == " ":
            if in_word:
                in_word = False
        else:
            if not in_word:
                in_word = True
                count = count + 1
        i = i + 1
    return count


def longest_run(s: str) -> int:
    if len(s) == 0:
        return 0
    max_run: int = 1
    current_run: int = 1
    i: int = 1
    while i < len(s):
        if s[i:i + 1] == s[i - 1:i]:
            current_run = current_run + 1
            if current_run > max_run:
                max_run = current_run
        else:
            current_run = 1
        i = i + 1
    return max_run


def test_basic_ops() -> None:
    print("=== Basic Operations ===")
    s: str = "Hello World"
    print("Original: " + s)
    print("Length: " + str(len(s)))
    print("Reversed: " + str_reverse(s))
    print("Upper: " + str_upper(s))
    print("Lower: " + str_lower(s))
    print("Char at 0: " + str_char_at(s, 0))
    print("Char at 4: " + str_char_at(s, 4))


def test_counting() -> None:
    print("=== Counting ===")
    s: str = "Hello World 123"
    print("Text: " + s)
    print("Vowels: " + str(count_vowels(s)))
    print("Consonants: " + str(count_consonants(s)))
    print("Digits: " + str(count_digits(s)))
    print("Spaces: " + str(count_spaces(s)))
    print("Words: " + str(word_count(s)))

    s2: str = "The quick brown fox jumps over the lazy dog"
    print("Text: " + s2)
    print("Vowels: " + str(count_vowels(s2)))
    print("Consonants: " + str(count_consonants(s2)))
    print("Words: " + str(word_count(s2)))


def test_palindrome() -> None:
    print("=== Palindrome ===")
    words: list = ["racecar", "hello", "madam", "level", "world", "noon", "abc"]
    i: int = 0
    while i < 7:
        w: str = words[i]
        if is_palindrome(w):
            print(w + ": palindrome")
        else:
            print(w + ": not palindrome")
        i = i + 1


def test_search() -> None:
    print("=== Search ===")
    text: str = "the cat sat on the mat and the cat ate the rat"
    print("Text: " + text)
    if str_contains(text, "cat"):
        print("Contains cat: yes")
    else:
        print("Contains cat: no")
    if str_contains(text, "dog"):
        print("Contains dog: yes")
    else:
        print("Contains dog: no")
    print("Index of cat: " + str(str_index_of(text, "cat")))
    print("Index of mat: " + str(str_index_of(text, "mat")))
    print("Index of dog: " + str(str_index_of(text, "dog")))
    print("Count the: " + str(str_count_occurrences(text, "the")))
    print("Count cat: " + str(str_count_occurrences(text, "cat")))
    print("Count at: " + str(str_count_occurrences(text, "at")))


def test_transform() -> None:
    print("=== Transform ===")
    s: str = "Hello World"
    print("Replace o->0: " + str_replace_char(s, "o", "0"))
    print("Replace l->L: " + str_replace_char(s, "l", "L"))
    padded: str = "   hello   "
    print("Trimmed: [" + str_trim(padded) + "]")
    print("Trim left: [" + str_trim_left(padded) + "]")
    print("Trim right: [" + str_trim_right(padded) + "]")
    print("Repeat ab x3: " + str_repeat("ab", 3))
    print("Pad left 42 to 8: [" + str_pad_left("42", 8, "0") + "]")
    print("Pad right hi to 8: [" + str_pad_right("hi", 8, ".") + "]")


def test_prefix_suffix() -> None:
    print("=== Prefix/Suffix ===")
    s: str = "Hello World"
    if str_starts_with(s, "Hello"):
        print("Starts with Hello: yes")
    if str_starts_with(s, "World"):
        print("Starts with World: yes")
    else:
        print("Starts with World: no")
    if str_ends_with(s, "World"):
        print("Ends with World: yes")
    if str_ends_with(s, "Hello"):
        print("Ends with Hello: yes")
    else:
        print("Ends with Hello: no")


def test_caesar() -> None:
    print("=== Caesar Cipher ===")
    plain: str = "Hello World"
    shift: int = 3
    encrypted: str = caesar_encrypt(plain, shift)
    decrypted: str = caesar_decrypt(encrypted, shift)
    print("Plain: " + plain)
    print("Encrypted shift 3: " + encrypted)
    print("Decrypted: " + decrypted)
    if plain == decrypted:
        print("Round trip: success")
    else:
        print("Round trip: failed")

    plain2: str = "The Quick Brown Fox"
    enc2: str = caesar_encrypt(plain2, 13)
    dec2: str = caesar_decrypt(enc2, 13)
    print("Plain: " + plain2)
    print("ROT13: " + enc2)
    print("Decoded: " + dec2)


def test_rle() -> None:
    print("=== Run-Length Encoding ===")
    tests: list = ["aaabbbccc", "abcdef", "aabbcc", "aaaaaaa", "abba"]
    i: int = 0
    while i < 5:
        s: str = tests[i]
        encoded: str = run_length_encode(s)
        print(s + " -> " + encoded)
        i = i + 1


def test_longest_run() -> None:
    print("=== Longest Run ===")
    tests: list = ["aabbbcccc", "abcdef", "aaaa", "aabbcc", "abccccba"]
    i: int = 0
    while i < 5:
        s: str = tests[i]
        print(s + ": longest run = " + str(longest_run(s)))
        i = i + 1


def main() -> int:
    test_basic_ops()
    test_counting()
    test_palindrome()
    test_search()
    test_transform()
    test_prefix_suffix()
    test_caesar()
    test_rle()
    test_longest_run()
    return 0


if __name__ == "__main__":
    main()
