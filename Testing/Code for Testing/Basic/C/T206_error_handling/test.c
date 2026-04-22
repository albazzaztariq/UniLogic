#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }
#include <setjmp.h>
typedef struct { const char* type; char message[256]; } __ul_exception_t;
static __ul_exception_t __ul_current_exception;
static jmp_buf* __ul_jmp_stack[64];
static int __ul_jmp_top = -1;
static void __ul_throw(const char* type, const char* msg) {
    __ul_current_exception.type = type;
    strncpy(__ul_current_exception.message, msg, 255);
    __ul_current_exception.message[255] = 0;
    if (__ul_jmp_top >= 0) { longjmp(*__ul_jmp_stack[__ul_jmp_top], 1); }
    else { fprintf(stderr, "unhandled exception: %s: %s\n", type, msg); exit(1); }
}
#include <stdlib.h>

typedef struct {
    int _ok;
    int _value;
    char* _error;
} _Result_int;

_Result_int safe_divide(int a, int b);
_Result_int chain_calc(int x);
int risky_divide(int a, int b);
int main(void);

_Result_int safe_divide(int a, int b)
{
    if ((b == 0)) {
        return (_Result_int){0, 0, "division by zero"};
    }
    return (_Result_int){1, (a / b), ""};
}

_Result_int chain_calc(int x)
{
    _Result_int _r0 = safe_divide(x, 2);
    if (!_r0._ok) {
        return (_Result_int){0, 0, _r0._error};
    }
    int half = _r0._value;
    int doubled = (half * 2);
    return (_Result_int){1, doubled, ""};
}

int risky_divide(int a, int b)
{
    if ((b == 0)) {
        __ul_throw("ArithmeticError", "cannot divide by zero");
    }
    return (a / b);
}

int main(void)
{
    {
        jmp_buf __ul_try_buf_0;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_0;
        if (setjmp(__ul_try_buf_0) == 0) {
            int r = risky_divide(10, 0);
            printf("%s\n", "should not reach");
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            if (strcmp(__ul_current_exception.type, "ArithmeticError") == 0) {
                const char* e = __ul_current_exception.message;
                printf("%s\n", __ul_strcat("caught: ", e));
            } else {
                __ul_throw(__ul_current_exception.type, __ul_current_exception.message);
            }
        }
    }
    {
        jmp_buf __ul_try_buf_1;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_1;
        if (setjmp(__ul_try_buf_1) == 0) {
            int r = risky_divide(10, 2);
            printf("%d\n", r);
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            if (strcmp(__ul_current_exception.type, "ArithmeticError") == 0) {
                printf("%s\n", "should not catch");
            } else {
                __ul_throw(__ul_current_exception.type, __ul_current_exception.message);
            }
        }
    }
    {
        jmp_buf __ul_try_buf_2;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_2;
        if (setjmp(__ul_try_buf_2) == 0) {
            printf("%s\n", "in try");
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
        }
        printf("%s\n", "in finally");
    }
    {
        jmp_buf __ul_try_buf_3;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_3;
        if (setjmp(__ul_try_buf_3) == 0) {
            __ul_throw("SomeError", "unknown problem");
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            {
                const char* e = __ul_current_exception.message;
                printf("%s\n", __ul_strcat("catch-all: ", e));
            }
        }
    }
    {
        jmp_buf __ul_try_buf_4;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_4;
        if (setjmp(__ul_try_buf_4) == 0) {
            __ul_throw("TestError", "test");
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            if (strcmp(__ul_current_exception.type, "TestError") == 0) {
                const char* e = __ul_current_exception.message;
                printf("%s\n", __ul_strcat("caught: ", e));
            } else {
                __ul_throw(__ul_current_exception.type, __ul_current_exception.message);
            }
        }
        printf("%s\n", "finally ran");
    }
    _Result_int _r1 = safe_divide(10, 2);
    if (!_r1._ok) {
        fprintf(stderr, "error: %s\n", _r1._error);
        exit(1);
    }
    int a = _r1._value;
    printf("%d\n", a);
    _Result_int _r2 = chain_calc(20);
    if (!_r2._ok) {
        fprintf(stderr, "error: %s\n", _r2._error);
        exit(1);
    }
    int b = _r2._value;
    printf("%d\n", b);
    return 0;
}


