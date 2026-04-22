"""Text processing utilities — word count, reverse, palindrome check."""


def reverse_string(s: str) -> str:
    result: str = ""
    i: int = len(s) - 1
    while i >= 0:
        result = result + s[i]
        i = i - 1
    return result


def is_palindrome(s: str) -> bool:
    return s == reverse_string(s)


def count_char(s: str, ch: str) -> int:
    count: int = 0
    i: int = 0
    while i < len(s):
        if s[i] == ch:
            count = count + 1
        i = i + 1
    return count


def to_upper(s: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        c: int = ord(s[i])
        if c >= 97 and c <= 122:
            result = result + chr(c - 32)
        else:
            result = result + s[i]
        i = i + 1
    return result


def repeat_string(s: str, times: int) -> str:
    result: str = ""
    i: int = 0
    while i < times:
        result = result + s
        i = i + 1
    return result


def main() -> int:
    # Reverse
    print(reverse_string("hello"))
    print(reverse_string("abcdef"))

    # Palindrome
    if is_palindrome("racecar"):
        print("racecar is palindrome")
    else:
        print("racecar is not palindrome")

    if is_palindrome("hello"):
        print("hello is palindrome")
    else:
        print("hello is not palindrome")

    # Count char
    print(count_char("mississippi", "s"))
    print(count_char("abcabc", "a"))

    # To upper
    print(to_upper("hello world"))
    print(to_upper("Python 3.12"))

    # Repeat
    print(repeat_string("ab", 4))
    print(repeat_string("xyz", 2))

    return 0


if __name__ == "__main__":
    main()
