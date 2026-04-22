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

int divide(int a, int b);
int risky(int x);
int main(void);

int divide(int a, int b)
{
    if ((b == 0)) {
        __ul_throw("DivisionByZero", "cannot divide by zero");
    }
    return (a / b);
}

int risky(int x)
{
    if ((x < 0)) {
        __ul_throw("ValueError", "negative input");
    }
    return (x * 2);
}

int main(void)
{
    printf("%d\n", divide(10, 2));
    {
        jmp_buf __ul_try_buf_0;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_0;
        if (setjmp(__ul_try_buf_0) == 0) {
            int result = divide(10, 0);
            printf("%d\n", result);
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            if (strcmp(__ul_current_exception.type, "DivisionByZero") == 0) {
                const char* err = __ul_current_exception.message;
                printf("%s\n", "caught: divide by zero");
            } else {
                __ul_throw(__ul_current_exception.type, __ul_current_exception.message);
            }
        }
    }
    {
        jmp_buf __ul_try_buf_1;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_1;
        if (setjmp(__ul_try_buf_1) == 0) {
            int val = risky((-5));
            printf("%d\n", val);
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            if (strcmp(__ul_current_exception.type, "ValueError") == 0) {
                const char* err = __ul_current_exception.message;
                printf("%s\n", "caught: negative input");
            } else {
                __ul_throw(__ul_current_exception.type, __ul_current_exception.message);
            }
        }
    }
    {
        jmp_buf __ul_try_buf_2;
        __ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_2;
        if (setjmp(__ul_try_buf_2) == 0) {
            int val = risky(10);
            printf("%d\n", val);
            __ul_jmp_top--;
        } else {
            __ul_jmp_top--;
            if (strcmp(__ul_current_exception.type, "ValueError") == 0) {
                const char* err = __ul_current_exception.message;
                printf("%s\n", "should not reach here");
            } else {
                __ul_throw(__ul_current_exception.type, __ul_current_exception.message);
            }
        }
    }
    return 0;
}


