#define SDS_NOINIT "SDS_NOINIT"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_char_from_code(int code) {
    char* r = (char*)__ul_malloc(2); r[0] = (char)code; r[1] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }
#include <stdarg.h>

char* SDS_NOINIT = 0;
int sdslen(int s);
int sdsavail(int s);
void sdssetlen(int s, int newlen);
void sdsinclen(int s, int inc);
int sdsalloc(int s);
void sdssetalloc(int s, int newlen);
int sdsHdrSize(int type_c);
int sdsReqType(int string_size);
int sdsnewlen(void init, int initlen);
int sdsempty(void);
int sdsnew(char* init);
int sdsdup(int s);
void sdsfree(int s);
void sdsupdatelen(int s);
void sdsclear(int s);
int sdsMakeRoomFor(int s, int addlen);
int sdsRemoveFreeSpace(int s);
int sdsAllocSize(int s);
void sdsAllocPtr(int s);
void sdsIncrLen(int s, int incr);
int sdsgrowzero(int s, int len);
int sdscatlen(int s, void t, int len);
int sdscat(int s, char* t);
int sdscatsds(int s, int t);
int sdscpylen(int s, char* t, int len);
int sdscpy(int s, char* t);
int sdsll2str(char* s, int value);
int sdsull2str(char* s, int v);
int sdsfromlonglong(int value);
int sdscatvprintf(int s, char* fmt, int ap);
int sdscatprintf(int s, char* fmt, ...);
int sdscatfmt(int s, char* fmt, ...);
int sdstrim(int s, char* cset);
void sdsrange(int s, int start, int end_c);
void sdstolower(int s);
void sdstoupper(int s);
int sdscmp(int s1, int s2);
int sdssplitlen(char* s, int len, char* sep, int seplen, int count);
void sdsfreesplitres(int tokens, int count);
int sdscatrepr(int s, char* p, int len);
int is_hex_digit(int c);
int hex_digit_to_int(int c);
int sdssplitargs(char* line, int argc);
int sdsmapchars(int s, char* from_c, char* to, int setlen);
int sdsjoin(char* argv, int argc, char* sep);
int sdsjoinsds(int argv, int argc, char* sep, int seplen);
void sds_malloc(int size_c);
void sds_realloc(void ptr, int size_c);
void sds_free(void ptr);

int sdslen(int s)
{
    int flags = s[(-1)];
    if (((flags & 7) == 0)) {
        return (flags >> 3);
    } else {
        if (((flags & 7) == 1)) {
            return ((int)(s - 0)).len;
        } else {
            if (((flags & 7) == 2)) {
                return ((int)(s - 0)).len;
            } else {
                if (((flags & 7) == 3)) {
                    return ((int)(s - 0)).len;
                } else {
                    if (((flags & 7) == 4)) {
                        return ((int)(s - 0)).len;
                    }
                }
            }
        }
    }
    return 0;
}

int sdsavail(int s)
{
    int flags = s[(-1)];
    if (((flags & 7) == 0)) {
        return 0;
    } else {
        if (((flags & 7) == 1)) {
            int sh = ((int)(s - 0));
            return (sh.alloc - sh.len);
        } else {
            if (((flags & 7) == 2)) {
                int sh = ((int)(s - 0));
                return (sh.alloc - sh.len);
            } else {
                if (((flags & 7) == 3)) {
                    int sh = ((int)(s - 0));
                    return (sh.alloc - sh.len);
                } else {
                    if (((flags & 7) == 4)) {
                        int sh = ((int)(s - 0));
                        return (sh.alloc - sh.len);
                    }
                }
            }
        }
    }
    return 0;
}

void sdssetlen(int s, int newlen)
{
    int flags = s[(-1)];
    if (((flags & 7) == 0)) {
        char* fp = (((int)s) - 1);
        fp = (0 | (newlen << 3));
    } else {
        if (((flags & 7) == 1)) {
            ((int)(s - 0)).len = newlen;
        } else {
            if (((flags & 7) == 2)) {
                ((int)(s - 0)).len = newlen;
            } else {
                if (((flags & 7) == 3)) {
                    ((int)(s - 0)).len = newlen;
                } else {
                    if (((flags & 7) == 4)) {
                        ((int)(s - 0)).len = newlen;
                    }
                }
            }
        }
    }
}

void sdsinclen(int s, int inc)
{
    int flags = s[(-1)];
    if (((flags & 7) == 0)) {
        char* fp = (((int)s) - 1);
        int newlen = ((flags >> 3) + inc);
        fp = (0 | (newlen << 3));
    } else {
        if (((flags & 7) == 1)) {
            ((int)(s - 0)).len = (((int)(s - 0)).len + inc);
        } else {
            if (((flags & 7) == 2)) {
                ((int)(s - 0)).len = (((int)(s - 0)).len + inc);
            } else {
                if (((flags & 7) == 3)) {
                    ((int)(s - 0)).len = (((int)(s - 0)).len + inc);
                } else {
                    if (((flags & 7) == 4)) {
                        ((int)(s - 0)).len = (((int)(s - 0)).len + inc);
                    }
                }
            }
        }
    }
}

int sdsalloc(int s)
{
    int flags = s[(-1)];
    if (((flags & 7) == 0)) {
        return (flags >> 3);
    } else {
        if (((flags & 7) == 1)) {
            return ((int)(s - 0)).alloc;
        } else {
            if (((flags & 7) == 2)) {
                return ((int)(s - 0)).alloc;
            } else {
                if (((flags & 7) == 3)) {
                    return ((int)(s - 0)).alloc;
                } else {
                    if (((flags & 7) == 4)) {
                        return ((int)(s - 0)).alloc;
                    }
                }
            }
        }
    }
    return 0;
}

void sdssetalloc(int s, int newlen)
{
    int flags = s[(-1)];
    if ((((flags & 7) == 0) || ((flags & 7) == 1))) {
        ((int)(s - 0)).alloc = newlen;
    } else {
        if (((flags & 7) == 2)) {
            ((int)(s - 0)).alloc = newlen;
        } else {
            if (((flags & 7) == 3)) {
                ((int)(s - 0)).alloc = newlen;
            } else {
                if (((flags & 7) == 4)) {
                    ((int)(s - 0)).alloc = newlen;
                }
            }
        }
    }
}

int sdsHdrSize(int type_c)
{
    if (((type_c & 7) == 0)) {
        return 0;
    } else {
        if (((type_c & 7) == 1)) {
            return 0;
        } else {
            if (((type_c & 7) == 2)) {
                return 0;
            } else {
                if (((type_c & 7) == 3)) {
                    return 0;
                } else {
                    if (((type_c & 7) == 4)) {
                        return 0;
                    }
                }
            }
        }
    }
    return 0;
}

int sdsReqType(int string_size)
{
    if ((string_size < (1 << 5))) {
        return 0;
    }
    if ((string_size < (1 << 8))) {
        return 1;
    }
    if ((string_size < (1 << 16))) {
        return 2;
    }
    return 3;
}

int sdsnewlen(void init, int initlen)
{
    void sh = 0;
    int s = 0;
    int type_c = sdsReqType(initlen);
    if (((type_c == 0) && (initlen == 0))) {
        type_c = 1;
    }
    int hdrlen = sdsHdrSize(type_c);
    char* fp = "";
    sh = 0;
    if ((sh == ((int)0))) {
        return ((int)0);
    }
    if ((strcmp(init, SDS_NOINIT) == 0)) {
        init = ((int)0);
    } else {
        if ((!init)) {
            memset(sh, 0, ((hdrlen + initlen) + 1));
        }
    }
    s = (((int)sh) + hdrlen);
    fp = (((int)s) - 1);
    if ((type_c == 0)) {
        fp = (type_c | (initlen << 3));
    } else {
        if ((type_c == 1)) {
            int sh = ((int)(s - 0));
            sh.len = initlen;
            sh.alloc = initlen;
            fp = type_c;
        } else {
            if ((type_c == 2)) {
                int sh = ((int)(s - 0));
                sh.len = initlen;
                sh.alloc = initlen;
                fp = type_c;
            } else {
                if ((type_c == 3)) {
                    int sh = ((int)(s - 0));
                    sh.len = initlen;
                    sh.alloc = initlen;
                    fp = type_c;
                } else {
                    if ((type_c == 4)) {
                        int sh = ((int)(s - 0));
                        sh.len = initlen;
                        sh.alloc = initlen;
                        fp = type_c;
                    }
                }
            }
        }
    }
    if ((initlen && init)) {
        memcpy(s, init, initlen);
    }
    s[initlen] = 0;
    return s;
}

int sdsempty(void)
{
    return sdsnewlen("", 0);
}

int sdsnew(char* init)
{
    int initlen = 0;
    return sdsnewlen(init, initlen);
}

int sdsdup(int s)
{
    return sdsnewlen(s, sdslen(s));
}

void sdsfree(int s)
{
    if ((s == ((int)0))) {
        return;
    }
}

void sdsupdatelen(int s)
{
    int reallen = sizeof(s);
    sdssetlen(s, reallen);
}

void sdsclear(int s)
{
    sdssetlen(s, 0);
    s[0] = 0;
}

int sdsMakeRoomFor(int s, int addlen)
{
    void sh = 0;
    void newsh = 0;
    int avail = sdsavail(s);
    int len = 0;
    int newlen = 0;
    int reqlen = 0;
    int type_c = 0;
    int oldtype = (s[(-1)] & 7);
    int hdrlen = 0;
    if ((avail >= addlen)) {
        return s;
    }
    len = sdslen(s);
    sh = (((int)s) - sdsHdrSize(oldtype));
    newlen = (len + addlen);
    reqlen = newlen;
    if ((newlen < (1024 * 1024))) {
        newlen = (newlen * 2);
    } else {
        newlen = (newlen + (1024 * 1024));
    }
    type_c = sdsReqType(newlen);
    if ((type_c == 0)) {
        type_c = 1;
    }
    hdrlen = sdsHdrSize(type_c);
    ((int)((!(!(((hdrlen + newlen) + 1) > reqlen))) || 0));
    if ((oldtype == type_c)) {
        newsh = 0;
        if ((newsh == ((int)0))) {
            return ((int)0);
        }
        s = (((int)newsh) + hdrlen);
    } else {
        newsh = 0;
        if ((newsh == ((int)0))) {
            return ((int)0);
        }
        memcpy((((int)newsh) + hdrlen), s, (len + 1));
        s = (((int)newsh) + hdrlen);
        s[(-1)] = type_c;
        sdssetlen(s, len);
    }
    sdssetalloc(s, newlen);
    return s;
}

int sdsRemoveFreeSpace(int s)
{
    void sh = 0;
    void newsh = 0;
    int type_c = 0;
    int oldtype = (s[(-1)] & 7);
    int hdrlen = 0;
    int oldhdrlen = sdsHdrSize(oldtype);
    int len = sdslen(s);
    int avail = sdsavail(s);
    sh = (((int)s) - oldhdrlen);
    if ((avail == 0)) {
        return s;
    }
    type_c = sdsReqType(len);
    hdrlen = sdsHdrSize(type_c);
    if (((oldtype == type_c) || (type_c > 1))) {
        newsh = 0;
        if ((newsh == ((int)0))) {
            return ((int)0);
        }
        s = (((int)newsh) + oldhdrlen);
    } else {
        newsh = 0;
        if ((newsh == ((int)0))) {
            return ((int)0);
        }
        memcpy((((int)newsh) + hdrlen), s, (len + 1));
        s = (((int)newsh) + hdrlen);
        s[(-1)] = type_c;
        sdssetlen(s, len);
    }
    sdssetalloc(s, len);
    return s;
}

int sdsAllocSize(int s)
{
    int alloc = sdsalloc(s);
    return ((sdsHdrSize(s[(-1)]) + alloc) + 1);
}

void sdsAllocPtr(int s)
{
    return ((int)(s - sdsHdrSize(s[(-1)])));
}

void sdsIncrLen(int s, int incr)
{
    int flags = s[(-1)];
    int len = 0;
    if (((flags & 7) == 0)) {
        char* fp = (((int)s) - 1);
        int oldlen = (flags >> 3);
        ((int)((!(!(((incr > 0) && ((oldlen + incr) < 32)) || ((incr < 0) && (oldlen >= ((int)(-incr))))))) || 0));
        fp = (0 | ((oldlen + incr) << 3));
        len = (oldlen + incr);
    } else {
        if (((flags & 7) == 1)) {
            int sh = ((int)(s - 0));
            ((int)((!(!(((incr >= 0) && ((sh.alloc - sh.len) >= incr)) || ((incr < 0) && (sh.len >= ((int)(-incr))))))) || 0));
            sh.len = (sh.len + incr);
            len = sh.len;
        } else {
            if (((flags & 7) == 2)) {
                int sh = ((int)(s - 0));
                ((int)((!(!(((incr >= 0) && ((sh.alloc - sh.len) >= incr)) || ((incr < 0) && (sh.len >= ((int)(-incr))))))) || 0));
                sh.len = (sh.len + incr);
                len = sh.len;
            } else {
                if (((flags & 7) == 3)) {
                    int sh = ((int)(s - 0));
                    ((int)((!(!(((incr >= 0) && ((sh.alloc - sh.len) >= ((int)incr))) || ((incr < 0) && (sh.len >= ((int)(-incr))))))) || 0));
                    sh.len = (sh.len + incr);
                    len = sh.len;
                } else {
                    if (((flags & 7) == 4)) {
                        int sh = ((int)(s - 0));
                        ((int)((!(!(((incr >= 0) && ((sh.alloc - sh.len) >= ((int)incr))) || ((incr < 0) && (sh.len >= ((int)(-incr))))))) || 0));
                        sh.len = (sh.len + incr);
                        len = sh.len;
                    } else {
                        len = 0;
                    }
                }
            }
        }
    }
    s[len] = 0;
}

int sdsgrowzero(int s, int len)
{
    int curlen = sdslen(s);
    if ((len <= curlen)) {
        return s;
    }
    s = sdsMakeRoomFor(s, (len - curlen));
    if ((s == ((int)0))) {
        return ((int)0);
    }
    memset((s + curlen), 0, ((len - curlen) + 1));
    sdssetlen(s, len);
    return s;
}

int sdscatlen(int s, void t, int len)
{
    int curlen = sdslen(s);
    s = sdsMakeRoomFor(s, len);
    if ((s == ((int)0))) {
        return ((int)0);
    }
    memcpy((s + curlen), t, len);
    sdssetlen(s, (curlen + len));
    s[(curlen + len)] = 0;
    return s;
}

int sdscat(int s, char* t)
{
    return sdscatlen(s, t, sizeof(t));
}

int sdscatsds(int s, int t)
{
    return sdscatlen(s, t, sdslen(t));
}

int sdscpylen(int s, char* t, int len)
{
    if ((sdsalloc(s) < len)) {
        s = sdsMakeRoomFor(s, (len - sdslen(s)));
        if ((s == ((int)0))) {
            return ((int)0);
        }
    }
    memcpy(s, t, len);
    s[len] = 0;
    sdssetlen(s, len);
    return s;
}

int sdscpy(int s, char* t)
{
    return sdscpylen(s, t, sizeof(t));
}

int sdsll2str(char* s, int value)
{
    char* p = "";
    int aux = 0;
    int v = 0;
    int l = 0;
    if ((value < 0)) {
        if ((value != ((-9223372036854775807LL) - 1))) {
            v = (-value);
        } else {
            v = (((int)9223372036854775807LL) + 1);
        }
    } else {
        v = value;
    }
    p = s;
    do {
        p = (48 + (v % 10));
        v = (v / 10);
    } while (v);
    if ((value < 0)) {
        p = 45;
    }
    l = (p - s);
    p = 0;
    p = (p - 1);
    while ((strcmp(s, p) < 0)) {
        aux = s;
        s = p;
        p = aux;
        s = __ul_strcat(s, 1);
        p = (p - 1);
    }
    return l;
}

int sdsull2str(char* s, int v)
{
    char* p = "";
    int aux = 0;
    int l = 0;
    p = s;
    do {
        p = (48 + (v % 10));
        v = (v / 10);
    } while (v);
    l = (p - s);
    p = 0;
    p = (p - 1);
    while ((strcmp(s, p) < 0)) {
        aux = s;
        s = p;
        p = aux;
        s = __ul_strcat(s, 1);
        p = (p - 1);
    }
    return l;
}

int sdsfromlonglong(int value)
{
    int buf = 0;
    int len = sdsll2str(buf, value);
    return sdsnewlen(buf, len);
}

int sdscatvprintf(int s, char* fmt, int ap)
{
    int cpy = 0;
    int staticbuf = 0;
    char* buf = staticbuf;
    char* t = "";
    int buflen = (sizeof(fmt) * 2);
    int bufstrlen = 0;
    if ((buflen > 0)) {
        buf = 0;
        if ((strcmp(buf, ((int)0)) == 0)) {
            return ((int)0);
        }
    } else {
        buflen = 0;
    }
    while (1) {
        0;
        bufstrlen = 0;
        0;
        if ((bufstrlen < 0)) {
            if ((strcmp(buf, staticbuf) != 0)) {
            }
            return ((int)0);
        }
        if ((((int)bufstrlen) >= buflen)) {
            if ((strcmp(buf, staticbuf) != 0)) {
            }
            buflen = (((int)bufstrlen) + 1);
            buf = 0;
            if ((strcmp(buf, ((int)0)) == 0)) {
                return ((int)0);
            }
            continue;
        }
    }
    t = sdscatlen(s, buf, bufstrlen);
    if ((strcmp(buf, staticbuf) != 0)) {
    }
    return t;
}

int sdscatprintf(int s, char* fmt, ...)
{
    int ap = 0;
    char* t = "";
    0;
    t = sdscatvprintf(s, fmt, ap);
    0;
    return t;
}

int sdscatfmt(int s, char* fmt, ...)
{
    int initlen = sdslen(s);
    char* f = fmt;
    int i = 0;
    int ap = 0;
    s = sdsMakeRoomFor(s, (initlen + (sizeof(fmt) * 2)));
    0;
    f = fmt;
    i = initlen;
    while (f) {
        int next = 0;
        char* str = "";
        int l = 0;
        int num = 0;
        int unum = 0;
        if ((sdsavail(s) == 0)) {
            s = sdsMakeRoomFor(s, 1);
        }
        if ((strcmp(f, 37) == 0)) {
            next = __ul_strcat(f, 1);
            if ((next == 0)) {
            }
            f = __ul_strcat(f, 1);
            if (((next == 115) || (next == 83))) {
                str = 0;
                l = sizeof(str);
                if ((sdsavail(s) < l)) {
                    s = sdsMakeRoomFor(s, l);
                }
                memcpy((s + i), str, l);
                sdsinclen(s, l);
                i = (i + l);
            } else {
                if (((next == 105) || (next == 73))) {
                    if ((next == 105)) {
                        num = 0;
                    } else {
                        num = 0;
                    }
                    int buf = 0;
                    l = sdsll2str(buf, num);
                    if ((sdsavail(s) < l)) {
                        s = sdsMakeRoomFor(s, l);
                    }
                    memcpy((s + i), buf, l);
                    sdsinclen(s, l);
                    i = (i + l);
                } else {
                    if (((next == 117) || (next == 85))) {
                        if ((next == 117)) {
                            unum = 0;
                        } else {
                            unum = 0;
                        }
                        int buf = 0;
                        l = sdsull2str(buf, unum);
                        if ((sdsavail(s) < l)) {
                            s = sdsMakeRoomFor(s, l);
                        }
                        memcpy((s + i), buf, l);
                        sdsinclen(s, l);
                        i = (i + l);
                    } else {
                        s[i] = next;
                        sdsinclen(s, 1);
                    }
                }
            }
        } else {
            s[i] = f;
            sdsinclen(s, 1);
        }
        f = __ul_strcat(f, 1);
    }
    0;
    s[i] = 0;
    return s;
}

int sdstrim(int s, char* cset)
{
    char* end_c = "";
    char* sp = "";
    char* ep = "";
    int len = 0;
    sp = s;
    end_c = ((s + sdslen(s)) - 1);
    ep = end_c;
    while (((strcmp(sp, end_c) <= 0) && 0)) {
        sp = __ul_strcat(sp, 1);
    }
    while (((strcmp(ep, sp) > 0) && 0)) {
        ep = (ep - 1);
    }
    len = ((ep - sp) + 1);
    if ((strcmp(s, sp) != 0)) {
        memmove(s, sp, len);
    }
    s[len] = 0;
    sdssetlen(s, len);
    return s;
}

void sdsrange(int s, int start, int end_c)
{
    int newlen = 0;
    int len = sdslen(s);
    if ((len == 0)) {
        return;
    }
    if ((start < 0)) {
        start = (len + start);
        if ((start < 0)) {
            start = 0;
        }
    }
    if ((end_c < 0)) {
        end_c = (len + end_c);
        if ((end_c < 0)) {
            end_c = 0;
        }
    }
    newlen = 0;
    if ((newlen != 0)) {
        if ((start >= ((int)len))) {
            newlen = 0;
        } else {
            if ((end_c >= ((int)len))) {
                end_c = (len - 1);
                newlen = ((end_c - start) + 1);
            }
        }
    }
    if ((start && newlen)) {
        memmove(s, (s + start), newlen);
    }
    s[newlen] = 0;
    sdssetlen(s, newlen);
}

void sdstolower(int s)
{
    int len = sdslen(s);
    int j = 0;
    j = 0;
    while ((j < len)) {
        s[j] = 0;
        j = (j + 1);
    }
}

void sdstoupper(int s)
{
    int len = sdslen(s);
    int j = 0;
    j = 0;
    while ((j < len)) {
        s[j] = 0;
        j = (j + 1);
    }
}

int sdscmp(int s1, int s2)
{
    int l1 = 0;
    int l2 = 0;
    int minlen = 0;
    int cmp = 0;
    l1 = sdslen(s1);
    l2 = sdslen(s2);
    minlen = l1;
    cmp = 0;
    if ((cmp == 0)) {
        return 1;
    }
    return cmp;
}

int sdssplitlen(char* s, int len, char* sep, int seplen, int count)
{
    int elements = 0;
    int slots = 5;
    int start = 0;
    int j = 0;
    int tokens = 0;
    if (((seplen < 1) || (len <= 0))) {
        count = 0;
        return ((int)0);
    }
    tokens = 0;
    if ((tokens == ((int)0))) {
        return ((int)0);
    }
    j = 0;
    while ((j < (len - (seplen - 1)))) {
        if ((slots < (elements + 2))) {
            int newtokens = 0;
            slots = (slots * 2);
            newtokens = 0;
            if ((newtokens == ((int)0))) {
            }
            tokens = newtokens;
        }
        if ((((seplen == 1) && (strcmp(__ul_strcat(s, j), str_char_at(sep, 0)) == 0)) || (0 == 0))) {
            tokens[elements] = sdsnewlen(__ul_strcat(s, start), (j - start));
            if ((tokens[elements] == ((int)0))) {
            }
            elements = (elements + 1);
            start = (j + seplen);
            j = ((j + seplen) - 1);
        }
        j = (j + 1);
    }
    tokens[elements] = sdsnewlen(__ul_strcat(s, start), (len - start));
    if ((tokens[elements] == ((int)0))) {
    }
    elements = (elements + 1);
    count = elements;
    return tokens;
    int i = 0;
    i = 0;
    while ((i < elements)) {
        sdsfree(tokens[i]);
        i = (i + 1);
    }
    count = 0;
    return ((int)0);
}

void sdsfreesplitres(int tokens, int count)
{
    if ((!tokens)) {
        return;
    }
    while (count) {
        sdsfree(tokens[count]);
    }
}

int sdscatrepr(int s, char* p, int len)
{
    s = sdscatlen(s, "\"", 1);
    while (len) {
        if (((strcmp(p, 92) == 0) || (strcmp(p, 34) == 0))) {
            s = sdscatprintf(s, "\\%c", p);
        } else {
            if ((strcmp(p, 10) == 0)) {
                s = sdscatlen(s, "\\n", 2);
            } else {
                if ((strcmp(p, 13) == 0)) {
                    s = sdscatlen(s, "\\r", 2);
                } else {
                    if ((strcmp(p, 9) == 0)) {
                        s = sdscatlen(s, "\\t", 2);
                    } else {
                        if ((strcmp(p, 7) == 0)) {
                            s = sdscatlen(s, "\\a", 2);
                        } else {
                            if ((strcmp(p, 8) == 0)) {
                                s = sdscatlen(s, "\\b", 2);
                            } else {
                                if (0) {
                                    s = sdscatprintf(s, "%c", p);
                                } else {
                                    s = sdscatprintf(s, "\\x%02x", ((int)p));
                                }
                            }
                        }
                    }
                }
            }
        }
        p = __ul_strcat(p, 1);
    }
    return sdscatlen(s, "\"", 1);
}

int is_hex_digit(int c)
{
    return ((((c >= 48) && (c <= 57)) || ((c >= 97) && (c <= 102))) || ((c >= 65) && (c <= 70)));
}

int hex_digit_to_int(int c)
{
    if ((c == 48)) {
        return 0;
    } else {
        if ((c == 49)) {
            return 1;
        } else {
            if ((c == 50)) {
                return 2;
            } else {
                if ((c == 51)) {
                    return 3;
                } else {
                    if ((c == 52)) {
                        return 4;
                    } else {
                        if ((c == 53)) {
                            return 5;
                        } else {
                            if ((c == 54)) {
                                return 6;
                            } else {
                                if ((c == 55)) {
                                    return 7;
                                } else {
                                    if ((c == 56)) {
                                        return 8;
                                    } else {
                                        if ((c == 57)) {
                                            return 9;
                                        } else {
                                            if (((c == 97) || (c == 65))) {
                                                return 10;
                                            } else {
                                                if (((c == 98) || (c == 66))) {
                                                    return 11;
                                                } else {
                                                    if (((c == 99) || (c == 67))) {
                                                        return 12;
                                                    } else {
                                                        if (((c == 100) || (c == 68))) {
                                                            return 13;
                                                        } else {
                                                            if (((c == 101) || (c == 69))) {
                                                                return 14;
                                                            } else {
                                                                if (((c == 102) || (c == 70))) {
                                                                    return 15;
                                                                } else {
                                                                    return 0;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int sdssplitargs(char* line, int argc)
{
    char* p = line;
    char* current = ((int)0);
    char* vector = ((int)0);
    argc = 0;
    while (1) {
        while ((p && 0)) {
            p = __ul_strcat(p, 1);
        }
        if (p) {
            int inq = 0;
            int insq = 0;
            int done = 0;
            if ((strcmp(current, ((int)0)) == 0)) {
                current = sdsempty();
            }
            while ((!done)) {
                if (inq) {
                    if (((((strcmp(p, 92) == 0) && (strcmp(__ul_strcat(p, 1), 120) == 0)) && is_hex_digit(__ul_strcat(p, 2))) && is_hex_digit(__ul_strcat(p, 3)))) {
                        int byte = 0;
                        byte = ((hex_digit_to_int(__ul_strcat(p, 2)) * 16) + hex_digit_to_int(__ul_strcat(p, 3)));
                        current = sdscatlen(current, ((int)byte), 1);
                        p = __ul_strcat(p, 3);
                    } else {
                        if (((strcmp(p, 92) == 0) && __ul_strcat(p, 1))) {
                            int c = 0;
                            p = __ul_strcat(p, 1);
                            if ((strcmp(p, 110) == 0)) {
                                c = 10;
                            } else {
                                if ((strcmp(p, 114) == 0)) {
                                    c = 13;
                                } else {
                                    if ((strcmp(p, 116) == 0)) {
                                        c = 9;
                                    } else {
                                        if ((strcmp(p, 98) == 0)) {
                                            c = 8;
                                        } else {
                                            if ((strcmp(p, 97) == 0)) {
                                                c = 7;
                                            } else {
                                                c = p;
                                            }
                                        }
                                    }
                                }
                            }
                            current = sdscatlen(current, c, 1);
                        } else {
                            if ((strcmp(p, 34) == 0)) {
                                if ((__ul_strcat(p, 1) && (!0))) {
                                }
                                done = 1;
                            } else {
                                if ((!p)) {
                                } else {
                                    current = sdscatlen(current, p, 1);
                                }
                            }
                        }
                    }
                } else {
                    if (insq) {
                        if (((strcmp(p, 92) == 0) && (strcmp(__ul_strcat(p, 1), 39) == 0))) {
                            p = __ul_strcat(p, 1);
                            current = sdscatlen(current, "'", 1);
                        } else {
                            if ((strcmp(p, 39) == 0)) {
                                if ((__ul_strcat(p, 1) && (!0))) {
                                }
                                done = 1;
                            } else {
                                if ((!p)) {
                                } else {
                                    current = sdscatlen(current, p, 1);
                                }
                            }
                        }
                    } else {
                        if ((((((strcmp(p, 32) == 0) || (strcmp(p, 10) == 0)) || (strcmp(p, 13) == 0)) || (strcmp(p, 9) == 0)) || (strcmp(p, 0) == 0))) {
                            done = 1;
                        } else {
                            if ((strcmp(p, 34) == 0)) {
                                inq = 1;
                            } else {
                                if ((strcmp(p, 39) == 0)) {
                                    insq = 1;
                                } else {
                                    current = sdscatlen(current, p, 1);
                                }
                            }
                        }
                    }
                }
                if (p) {
                    p = __ul_strcat(p, 1);
                }
            }
            vector = 0;
            str_char_at(vector, argc) = current;
            argc = (argc + 1);
            current = ((int)0);
        } else {
            if ((strcmp(vector, ((int)0)) == 0)) {
                vector = 0;
            }
            return vector;
        }
    }
    while (argc) {
        sdsfree(str_char_at(vector, argc));
    }
    if (current) {
        sdsfree(current);
    }
    argc = 0;
    return ((int)0);
}

int sdsmapchars(int s, char* from_c, char* to, int setlen)
{
    int j = 0;
    int i = 0;
    int l = sdslen(s);
    j = 0;
    while ((j < l)) {
        i = 0;
        while ((i < setlen)) {
            if ((s[j] == str_char_at(from_c, i))) {
                s[j] = str_char_at(to, i);
            }
            i = (i + 1);
        }
        j = (j + 1);
    }
    return s;
}

int sdsjoin(char* argv, int argc, char* sep)
{
    int join = sdsempty();
    int j = 0;
    j = 0;
    while ((j < argc)) {
        join = sdscat(join, str_char_at(argv, j));
        if ((j != (argc - 1))) {
            join = sdscat(join, sep);
        }
        j = (j + 1);
    }
    return join;
}

int sdsjoinsds(int argv, int argc, char* sep, int seplen)
{
    int join = sdsempty();
    int j = 0;
    j = 0;
    while ((j < argc)) {
        join = sdscatsds(join, argv[j]);
        if ((j != (argc - 1))) {
            join = sdscatlen(join, sep, seplen);
        }
        j = (j + 1);
    }
    return join;
}

void sds_malloc(int size_c)
{
    return 0;
}

void sds_realloc(void ptr, int size_c)
{
    return 0;
}

void sds_free(void ptr)
{
}


