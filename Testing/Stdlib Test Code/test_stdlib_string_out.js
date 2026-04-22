function __ul_to_str(v) {
    if (v instanceof Uint8Array) {
        let end = v.indexOf(0); if (end < 0) end = v.length;
        return String.fromCharCode(...v.slice(0, end));
    }
    return v;
}

function strcat(dest, src) { let e = dest.indexOf(0); if (e < 0) e = dest.length; for (let i = 0; i < src.length; i++) dest[e+i] = src.charCodeAt(i); dest[e+src.length] = 0; return dest; }
function strcmp(a, b) { return a < b ? -1 : a > b ? 1 : 0; }
function strcpy(dest, src) { for (let i = 0; i < src.length; i++) dest[i] = src.charCodeAt(i); dest[src.length] = 0; return dest; }
function strlen(s) { return s.length; }
function strncmp(a, b, n) { const sa = a.slice(0, n), sb = b.slice(0, n); return sa < sb ? -1 : sa > sb ? 1 : 0; }
function strstr(s, sub) { const i = s.indexOf(sub); return i >= 0 ? s.slice(i) : null; }

function str_len(s) {
    return strlen(s);
}

function str_equals(a, b) {
    return (strcmp(a, b) === 0);
}

function str_contains(s, sub) {
    let found = strstr(s, sub);
    return (found !== null);
}

function str_starts_with(s, prefix) {
    let plen = strlen(prefix);
    return (strncmp(s, prefix, plen) === 0);
}

function str_ends_with(s, suffix) {
    let slen = strlen(s);
    let suflen = strlen(suffix);
    if ((suflen > slen)) {
        return false;
    }
    let offset = (slen - suflen);
    let i = 0;
    while ((i < suflen)) {
        if ((s[(offset + i)] !== suffix[i])) {
            return false;
        }
        i = (i + 1);
    }
    return true;
}

function str_upper(s) {
    let len = strlen(s);
    let result = new Uint8Array((len + 1));
    let i = 0;
    while ((i < len)) {
        let c = s.charCodeAt(i);
        if ((c >= 97)) {
            if ((c <= 122)) {
                c = (c - 32);
            }
        }
        result[i] = c;
        i = (i + 1);
    }
    result[len] = 0;
    return __ul_to_str(result);
}

function str_lower(s) {
    let len = strlen(s);
    let result = new Uint8Array((len + 1));
    let i = 0;
    while ((i < len)) {
        let c = s.charCodeAt(i);
        if ((c >= 65)) {
            if ((c <= 90)) {
                c = (c + 32);
            }
        }
        result[i] = c;
        i = (i + 1);
    }
    result[len] = 0;
    return __ul_to_str(result);
}

function str_trim(s) {
    let len = strlen(s);
    let start = 0;
    while ((start < len)) {
        let c = s.charCodeAt(start);
        if ((c !== 32)) {
            if ((c !== 9)) {
                if ((c !== 10)) {
                    if ((c !== 13)) {
                        break;
                    }
                }
            }
        }
        start = (start + 1);
    }
    let stop = (len - 1);
    while ((stop >= start)) {
        let c = s.charCodeAt(stop);
        if ((c !== 32)) {
            if ((c !== 9)) {
                if ((c !== 10)) {
                    if ((c !== 13)) {
                        break;
                    }
                }
            }
        }
        stop = (stop - 1);
    }
    let newlen = ((stop - start) + 1);
    let result = new Uint8Array((newlen + 1));
    let i = 0;
    while ((i < newlen)) {
        result[i] = s.charCodeAt((start + i));
        i = (i + 1);
    }
    result[newlen] = 0;
    return __ul_to_str(result);
}

function str_concat(a, b) {
    let alen = strlen(a);
    let blen = strlen(b);
    let result = new Uint8Array(((alen + blen) + 1));
    strcpy(result, a);
    strcat(result, b);
    return __ul_to_str(result);
}

function main() {
    let len = str_len("hello");
    console.log(len);
    let eq = str_equals("abc", "abc");
    console.log(eq);
    let neq = str_equals("abc", "xyz");
    console.log(neq);
    let has = str_contains("hello world", "world");
    console.log(has);
    let nope = str_contains("hello world", "xyz");
    console.log(nope);
    let sw = str_starts_with("hello world", "hello");
    console.log(sw);
    let ew = str_ends_with("hello world", "world");
    console.log(ew);
    let up = str_upper("hello");
    console.log(up);
    let lo = str_lower("HELLO");
    console.log(lo);
    let trimmed = str_trim("  hello  ");
    console.log(trimmed);
    let joined = str_concat("hello ", "world");
    console.log(joined);
    return 0;
}


main();
