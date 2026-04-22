// @dr python_compat = true

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

char* HOOKS[] = {"response", "error", "redirect"};
void default_hooks(void);
void dispatch_hook(int key, int hooks, int hook_data);
void add_one(int x);
void _py_double(int x);
int main(void);

void default_hooks(void)
{
    map _dict0 = map_new();
    __auto_type result = _dict0;
    for (int _i = 0; _i < (int)(sizeof(HOOKS)/sizeof(HOOKS[0])); _i++) {
        char* event = HOOKS[_i];
        result[event] = /* unknown expr ArrayLiteral */;
    }
    return result;
}

void dispatch_hook(int key, int hooks, int hook_data)
{
    if ((!hooks)) {
        return hook_data;
    }
    __auto_type handler_list = hooks._vtable->get(&hooks, key);
    if (handler_list) {
        for (int _i = 0; _i < (int)(sizeof(handler_list)/sizeof(handler_list[0])); _i++) {
            int hook = handler_list[_i];
            __auto_type new_data = hook(hook_data);
            if ((new_data != NULL)) {
                hook_data = new_data;
            }
        }
    }
    return hook_data;
}

void add_one(int x)
{
    return (x + 1);
}

void _py_double(int x)
{
    return (x * 2);
}

int main(void)
{
    hooks = default_hooks();
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%s", "default_hooks keys:");
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", _py_list(hooks._vtable->keys(&hooks)));
    printf("%s\n", __ul_strcat(__ul_strcat(_cast_buf_0, " "), _cast_buf_1));
    hooks["response"]._vtable->append(&hooks["response"], add_one);
    hooks["response"]._vtable->append(&hooks["response"], _py_double);
    __auto_type result = dispatch_hook("response", hooks, 5);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%s", "dispatch_hook result:");
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", result);
    printf("%s\n", __ul_strcat(__ul_strcat(_cast_buf_2, " "), _cast_buf_3));
    __auto_type result2 = dispatch_hook("error", hooks, 99);
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%s", "dispatch_hook empty key:");
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", result2);
    printf("%s\n", __ul_strcat(__ul_strcat(_cast_buf_4, " "), _cast_buf_5));
    __auto_type result3 = dispatch_hook("response", NULL, 10);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%s", "dispatch_hook None hooks:");
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", result3);
    printf("%s\n", __ul_strcat(__ul_strcat(_cast_buf_6, " "), _cast_buf_7));
    return 0;
}


