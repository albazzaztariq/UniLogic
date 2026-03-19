#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* str_substr(const char* s, int start, int len) {
    int slen = (int)strlen(s); if (start < 0) start = 0; if (start >= slen) { static char e[1] = {0}; return e; }
    if (len > slen - start) len = slen - start; char* r = (char*)__ul_malloc(len + 1); memcpy(r, s + start, len); r[len] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }
#include <stdlib.h>

typedef struct {
    int _ok;
    int _value;
    char* _error;
} _Result_int;

void skip_spaces(char* expr, int* pos);
_Result_int parse_number(char* expr, int* pos);
_Result_int parse_primary(char* expr, int* pos);
_Result_int parse_mul(char* expr, int* pos);
_Result_int parse_add(char* expr, int* pos);
_Result_int evaluate(char* expr);
void test_eval(char* expr);
int main(void);

void skip_spaces(char* expr, int* pos)
{
    int len = strlen(expr);
    while ((pos[0] < len)) {
        char* ch = str_char_at(expr, pos[0]);
        if ((strcmp(ch, " ") == 0)) {
            pos[0] = (pos[0] + 1);
        } else {
            break;
        }
    }
}

_Result_int parse_number(char* expr, int* pos)
{
    int len = strlen(expr);
    skip_spaces(expr, pos);
    if ((pos[0] >= len)) {
        return (_Result_int){0, 0, "unexpected end of expression"};
    }
    int start = pos[0];
    int negative = 0;
    char* ch = str_char_at(expr, pos[0]);
    if ((strcmp(ch, "-") == 0)) {
        negative = 1;
        pos[0] = (pos[0] + 1);
        if ((pos[0] >= len)) {
            return (_Result_int){0, 0, "unexpected end after minus"};
        }
        ch = str_char_at(expr, pos[0]);
    }
    if ((strcmp(ch, "0") < 0)) {
        return (_Result_int){0, 0, "expected number"};
    }
    if ((strcmp(ch, "9") > 0)) {
        return (_Result_int){0, 0, "expected number"};
    }
    int result = 0;
    while ((pos[0] < len)) {
        ch = str_char_at(expr, pos[0]);
        if ((strcmp(ch, "0") >= 0)) {
            if ((strcmp(ch, "9") <= 0)) {
                int digit = 0;
                if ((strcmp(ch, "0") == 0)) {
                    digit = 0;
                }
                if ((strcmp(ch, "1") == 0)) {
                    digit = 1;
                }
                if ((strcmp(ch, "2") == 0)) {
                    digit = 2;
                }
                if ((strcmp(ch, "3") == 0)) {
                    digit = 3;
                }
                if ((strcmp(ch, "4") == 0)) {
                    digit = 4;
                }
                if ((strcmp(ch, "5") == 0)) {
                    digit = 5;
                }
                if ((strcmp(ch, "6") == 0)) {
                    digit = 6;
                }
                if ((strcmp(ch, "7") == 0)) {
                    digit = 7;
                }
                if ((strcmp(ch, "8") == 0)) {
                    digit = 8;
                }
                if ((strcmp(ch, "9") == 0)) {
                    digit = 9;
                }
                result = ((result * 10) + digit);
                pos[0] = (pos[0] + 1);
            } else {
                break;
            }
        } else {
            break;
        }
    }
    if (negative) {
        result = (0 - result);
    }
    return (_Result_int){1, result, ""};
}

_Result_int parse_primary(char* expr, int* pos)
{
    skip_spaces(expr, pos);
    int len = strlen(expr);
    if ((pos[0] >= len)) {
        return (_Result_int){0, 0, "unexpected end of expression"};
    }
    char* ch = str_char_at(expr, pos[0]);
    if ((strcmp(ch, "(") == 0)) {
        pos[0] = (pos[0] + 1);
        _Result_int _r0 = parse_add(expr, pos);
        if (!_r0._ok) {
            return (_Result_int){0, 0, _r0._error};
        }
        int val = _r0._value;
        skip_spaces(expr, pos);
        if ((pos[0] >= len)) {
            return (_Result_int){0, 0, "missing closing parenthesis"};
        }
        char* close = str_char_at(expr, pos[0]);
        if ((strcmp(close, ")") != 0)) {
            return (_Result_int){0, 0, "expected closing parenthesis"};
        }
        pos[0] = (pos[0] + 1);
        return (_Result_int){1, val, ""};
    }
    _Result_int _r1 = parse_number(expr, pos);
    if (!_r1._ok) {
        return (_Result_int){0, 0, _r1._error};
    }
    int num = _r1._value;
    return (_Result_int){1, num, ""};
}

_Result_int parse_mul(char* expr, int* pos)
{
    _Result_int _r2 = parse_primary(expr, pos);
    if (!_r2._ok) {
        return (_Result_int){0, 0, _r2._error};
    }
    int lhs = _r2._value;
    while ((pos[0] < strlen(expr))) {
        skip_spaces(expr, pos);
        if ((pos[0] >= strlen(expr))) {
            break;
        }
        char* op = str_char_at(expr, pos[0]);
        if ((strcmp(op, "*") == 0)) {
            pos[0] = (pos[0] + 1);
            _Result_int _r3 = parse_primary(expr, pos);
            if (!_r3._ok) {
                return (_Result_int){0, 0, _r3._error};
            }
            int rhs = _r3._value;
            lhs = (lhs * rhs);
        } else {
            if ((strcmp(op, "/") == 0)) {
                pos[0] = (pos[0] + 1);
                _Result_int _r4 = parse_primary(expr, pos);
                if (!_r4._ok) {
                    return (_Result_int){0, 0, _r4._error};
                }
                int rhs = _r4._value;
                if ((rhs == 0)) {
                    return (_Result_int){0, 0, "division by zero"};
                }
                lhs = (lhs / rhs);
            } else {
                break;
            }
        }
    }
    return (_Result_int){1, lhs, ""};
}

_Result_int parse_add(char* expr, int* pos)
{
    _Result_int _r5 = parse_mul(expr, pos);
    if (!_r5._ok) {
        return (_Result_int){0, 0, _r5._error};
    }
    int lhs = _r5._value;
    while ((pos[0] < strlen(expr))) {
        skip_spaces(expr, pos);
        if ((pos[0] >= strlen(expr))) {
            break;
        }
        char* op = str_char_at(expr, pos[0]);
        if ((strcmp(op, "+") == 0)) {
            pos[0] = (pos[0] + 1);
            _Result_int _r6 = parse_mul(expr, pos);
            if (!_r6._ok) {
                return (_Result_int){0, 0, _r6._error};
            }
            int rhs = _r6._value;
            lhs = (lhs + rhs);
        } else {
            if ((strcmp(op, "-") == 0)) {
                pos[0] = (pos[0] + 1);
                _Result_int _r7 = parse_mul(expr, pos);
                if (!_r7._ok) {
                    return (_Result_int){0, 0, _r7._error};
                }
                int rhs = _r7._value;
                lhs = (lhs - rhs);
            } else {
                break;
            }
        }
    }
    return (_Result_int){1, lhs, ""};
}

_Result_int evaluate(char* expr)
{
    int pos[] = {0};
    _Result_int _r8 = parse_add(expr, pos);
    if (!_r8._ok) {
        return (_Result_int){0, 0, _r8._error};
    }
    int result = _r8._value;
    return (_Result_int){1, result, ""};
}

void test_eval(char* expr)
{
    printf("%s\n", expr);
    _Result_int _r9 = evaluate(expr);
    if (!_r9._ok) {
        fprintf(stderr, "error: %s\n", _r9._error);
        exit(1);
    }
    int result = _r9._value;
    printf("%d\n", result);
    printf("%s\n", "");
}

int main(void)
{
    test_eval("3 + 4");
    test_eval("10 - 3");
    test_eval("6 * 7");
    test_eval("20 / 4");
    test_eval("3 + 4 * 2");
    test_eval("(3 + 4) * 2");
    test_eval("(10 + 2) * (8 - 3)");
    test_eval("100 - 20 * 3 + 5");
    test_eval("((2 + 3) * (4 + 1))");
    test_eval("42");
    return 0;
}


