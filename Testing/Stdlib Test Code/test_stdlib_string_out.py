from dataclasses import dataclass, field
def strcat(dest, src):
    enc = src.encode() if isinstance(src, str) else src
    try: end = dest.index(0)
    except ValueError: end = len(dest)
    dest[end:end+len(enc)] = enc; dest[end+len(enc)] = 0
    return dest
def strcmp(a, b): return (a > b) - (a < b)
def strcpy(dest, src):
    enc = src.encode() if isinstance(src, str) else src
    dest[:len(enc)] = enc; dest[len(enc)] = 0
    return dest
def strlen(s): return len(s)
def strncmp(a, b, n): return (a[:n] > b[:n]) - (a[:n] < b[:n])
def strstr(s, sub): idx = s.find(sub); return s[idx:] if idx >= 0 else None

def __ul_to_str(v):
    if isinstance(v, (bytearray, bytes)):
        idx = v.find(0)
        return v[:idx].decode() if idx >= 0 else v.decode()
    return v


def str_len(s):
    return strlen(s)

def str_equals(a, b):
    return (strcmp(a, b) == 0)

def str_contains(s, sub):
    found = strstr(s, sub)
    return (found != None)

def str_starts_with(s, prefix):
    plen = strlen(prefix)
    return (strncmp(s, prefix, plen) == 0)

def str_ends_with(s, suffix):
    slen = strlen(s)
    suflen = strlen(suffix)
    if (suflen > slen):
        return False
    offset = (slen - suflen)
    i = 0
    while (i < suflen):
        if (s[(offset + i)] != suffix[i]):
            return False
        i = (i + 1)
    return True

def str_upper(s):
    len = strlen(s)
    result = bytearray((len + 1))
    i = 0
    while (i < len):
        c = ord(s[i])
        if (c >= 97):
            if (c <= 122):
                c = (c - 32)
        result[i] = c
        i = (i + 1)
    result[len] = 0
    return __ul_to_str(result)

def str_lower(s):
    len = strlen(s)
    result = bytearray((len + 1))
    i = 0
    while (i < len):
        c = ord(s[i])
        if (c >= 65):
            if (c <= 90):
                c = (c + 32)
        result[i] = c
        i = (i + 1)
    result[len] = 0
    return __ul_to_str(result)

def str_trim(s):
    len = strlen(s)
    start = 0
    while (start < len):
        c = ord(s[start])
        if (c != 32):
            if (c != 9):
                if (c != 10):
                    if (c != 13):
                        break
        start = (start + 1)
    stop = (len - 1)
    while (stop >= start):
        c = ord(s[stop])
        if (c != 32):
            if (c != 9):
                if (c != 10):
                    if (c != 13):
                        break
        stop = (stop - 1)
    newlen = ((stop - start) + 1)
    result = bytearray((newlen + 1))
    i = 0
    while (i < newlen):
        result[i] = ord(s[(start + i)])
        i = (i + 1)
    result[newlen] = 0
    return __ul_to_str(result)

def str_concat(a, b):
    alen = strlen(a)
    blen = strlen(b)
    result = bytearray(((alen + blen) + 1))
    strcpy(result, a)
    strcat(result, b)
    return __ul_to_str(result)

def main():
    len = str_len('hello')
    print(len)
    eq = str_equals('abc', 'abc')
    print(eq)
    neq = str_equals('abc', 'xyz')
    print(neq)
    has = str_contains('hello world', 'world')
    print(has)
    nope = str_contains('hello world', 'xyz')
    print(nope)
    sw = str_starts_with('hello world', 'hello')
    print(sw)
    ew = str_ends_with('hello world', 'world')
    print(ew)
    up = str_upper('hello')
    print(up)
    lo = str_lower('HELLO')
    print(lo)
    trimmed = str_trim('  hello  ')
    print(trimmed)
    joined = str_concat('hello ', 'world')
    print(joined)
    return 0


if __name__ == "__main__":
    main()
