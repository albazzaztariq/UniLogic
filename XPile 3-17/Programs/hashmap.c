// @dr memory = manual

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
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
    char* _value;
    char* _error;
} _Result_char_ptr;

int char_val(char* ch);
int hash_key(char* key);
void hm_set(char** keys, char** vals, int* hm_state, char* key, char* val);
_Result_char_ptr hm_get(char** keys, char** vals, int* hm_state, char* key);
int hm_has(char** keys, int* hm_state, char* key);
void hm_delete(char** keys, char** vals, int* hm_state, char* key);
int hm_len(int* hm_state);
int main(void);

int char_val(char* ch)
{
    if ((strcmp(ch, "a") == 0)) {
        return 97;
    }
    if ((strcmp(ch, "b") == 0)) {
        return 98;
    }
    if ((strcmp(ch, "c") == 0)) {
        return 99;
    }
    if ((strcmp(ch, "d") == 0)) {
        return 100;
    }
    if ((strcmp(ch, "e") == 0)) {
        return 101;
    }
    if ((strcmp(ch, "f") == 0)) {
        return 102;
    }
    if ((strcmp(ch, "g") == 0)) {
        return 103;
    }
    if ((strcmp(ch, "h") == 0)) {
        return 104;
    }
    if ((strcmp(ch, "i") == 0)) {
        return 105;
    }
    if ((strcmp(ch, "j") == 0)) {
        return 106;
    }
    if ((strcmp(ch, "k") == 0)) {
        return 107;
    }
    if ((strcmp(ch, "l") == 0)) {
        return 108;
    }
    if ((strcmp(ch, "m") == 0)) {
        return 109;
    }
    if ((strcmp(ch, "n") == 0)) {
        return 110;
    }
    if ((strcmp(ch, "o") == 0)) {
        return 111;
    }
    if ((strcmp(ch, "p") == 0)) {
        return 112;
    }
    if ((strcmp(ch, "q") == 0)) {
        return 113;
    }
    if ((strcmp(ch, "r") == 0)) {
        return 114;
    }
    if ((strcmp(ch, "s") == 0)) {
        return 115;
    }
    if ((strcmp(ch, "t") == 0)) {
        return 116;
    }
    if ((strcmp(ch, "u") == 0)) {
        return 117;
    }
    if ((strcmp(ch, "v") == 0)) {
        return 118;
    }
    if ((strcmp(ch, "w") == 0)) {
        return 119;
    }
    if ((strcmp(ch, "x") == 0)) {
        return 120;
    }
    if ((strcmp(ch, "y") == 0)) {
        return 121;
    }
    if ((strcmp(ch, "z") == 0)) {
        return 122;
    }
    if ((strcmp(ch, "A") == 0)) {
        return 65;
    }
    if ((strcmp(ch, "B") == 0)) {
        return 66;
    }
    if ((strcmp(ch, "C") == 0)) {
        return 67;
    }
    if ((strcmp(ch, "D") == 0)) {
        return 68;
    }
    if ((strcmp(ch, "E") == 0)) {
        return 69;
    }
    if ((strcmp(ch, "F") == 0)) {
        return 70;
    }
    if ((strcmp(ch, "G") == 0)) {
        return 71;
    }
    if ((strcmp(ch, "H") == 0)) {
        return 72;
    }
    if ((strcmp(ch, "I") == 0)) {
        return 73;
    }
    if ((strcmp(ch, "J") == 0)) {
        return 74;
    }
    if ((strcmp(ch, "K") == 0)) {
        return 75;
    }
    if ((strcmp(ch, "L") == 0)) {
        return 76;
    }
    if ((strcmp(ch, "M") == 0)) {
        return 77;
    }
    if ((strcmp(ch, "N") == 0)) {
        return 78;
    }
    if ((strcmp(ch, "O") == 0)) {
        return 79;
    }
    if ((strcmp(ch, "P") == 0)) {
        return 80;
    }
    if ((strcmp(ch, "Q") == 0)) {
        return 81;
    }
    if ((strcmp(ch, "R") == 0)) {
        return 82;
    }
    if ((strcmp(ch, "S") == 0)) {
        return 83;
    }
    if ((strcmp(ch, "T") == 0)) {
        return 84;
    }
    if ((strcmp(ch, "U") == 0)) {
        return 85;
    }
    if ((strcmp(ch, "V") == 0)) {
        return 86;
    }
    if ((strcmp(ch, "W") == 0)) {
        return 87;
    }
    if ((strcmp(ch, "X") == 0)) {
        return 88;
    }
    if ((strcmp(ch, "Y") == 0)) {
        return 89;
    }
    if ((strcmp(ch, "Z") == 0)) {
        return 90;
    }
    if ((strcmp(ch, "0") == 0)) {
        return 48;
    }
    if ((strcmp(ch, "1") == 0)) {
        return 49;
    }
    if ((strcmp(ch, "2") == 0)) {
        return 50;
    }
    if ((strcmp(ch, "3") == 0)) {
        return 51;
    }
    if ((strcmp(ch, "4") == 0)) {
        return 52;
    }
    if ((strcmp(ch, "5") == 0)) {
        return 53;
    }
    if ((strcmp(ch, "6") == 0)) {
        return 54;
    }
    if ((strcmp(ch, "7") == 0)) {
        return 55;
    }
    if ((strcmp(ch, "8") == 0)) {
        return 56;
    }
    if ((strcmp(ch, "9") == 0)) {
        return 57;
    }
    if ((strcmp(ch, " ") == 0)) {
        return 32;
    }
    if ((strcmp(ch, "_") == 0)) {
        return 95;
    }
    if ((strcmp(ch, "-") == 0)) {
        return 45;
    }
    if ((strcmp(ch, ".") == 0)) {
        return 46;
    }
    return 0;
}

int hash_key(char* key)
{
    int hash = 5381;
    int len = strlen(key);
    for (int i = 0; i < len; i++) {
        char* ch = str_char_at(key, i);
        hash = ((hash * 31) + char_val(ch));
    }
    if ((hash < 0)) {
        hash = (0 - hash);
    }
    return hash;
}

void hm_set(char** keys, char** vals, int* hm_state, char* key, char* val)
{
    int cap = hm_state[0];
    if ((hm_state[1] >= (cap / 2))) {
        int new_cap = (cap * 2);
        char* old_keys[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
        char* old_vals[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
        for (int i = 0; i < cap; i++) {
            old_keys[i] = keys[i];
            old_vals[i] = vals[i];
            keys[i] = "";
            vals[i] = "";
        }
        hm_state[0] = new_cap;
        hm_state[1] = 0;
        for (int i = 0; i < cap; i++) {
            if ((strcmp(old_keys[i], "") != 0)) {
                if ((strcmp(old_keys[i], "__deleted__") != 0)) {
                    hm_set(keys, vals, hm_state, old_keys[i], old_vals[i]);
                }
            }
        }
        cap = new_cap;
    }
    int hash = hash_key(key);
    int index = (hash % cap);
    while ((strcmp(keys[index], "") != 0)) {
        if ((strcmp(keys[index], key) == 0)) {
            vals[index] = val;
            return;
        }
        if ((strcmp(keys[index], "__deleted__") == 0)) {
            break;
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
    keys[index] = key;
    vals[index] = val;
    hm_state[1] = (hm_state[1] + 1);
}

_Result_char_ptr hm_get(char** keys, char** vals, int* hm_state, char* key)
{
    int cap = hm_state[0];
    int hash = hash_key(key);
    int index = (hash % cap);
    while ((strcmp(keys[index], "") != 0)) {
        if ((strcmp(keys[index], key) == 0)) {
            return (_Result_char_ptr){1, vals[index], ""};
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
    return (_Result_char_ptr){0, 0, "key not found"};
}

int hm_has(char** keys, int* hm_state, char* key)
{
    int cap = hm_state[0];
    int hash = hash_key(key);
    int index = (hash % cap);
    while ((strcmp(keys[index], "") != 0)) {
        if ((strcmp(keys[index], key) == 0)) {
            return 1;
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
    return 0;
}

void hm_delete(char** keys, char** vals, int* hm_state, char* key)
{
    int cap = hm_state[0];
    int hash = hash_key(key);
    int index = (hash % cap);
    while ((strcmp(keys[index], "") != 0)) {
        if ((strcmp(keys[index], key) == 0)) {
            keys[index] = "__deleted__";
            vals[index] = "";
            hm_state[1] = (hm_state[1] - 1);
            return;
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
}

int hm_len(int* hm_state)
{
    return hm_state[1];
}

int main(void)
{
    char* keys[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
    char* vals[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
    int hm_state[] = {16, 0};
    printf("%s\n", "Setting 10 entries...");
    hm_set(keys, vals, hm_state, "apple", "red");
    hm_set(keys, vals, hm_state, "banana", "yellow");
    hm_set(keys, vals, hm_state, "cherry", "red");
    hm_set(keys, vals, hm_state, "date", "brown");
    hm_set(keys, vals, hm_state, "elderberry", "purple");
    hm_set(keys, vals, hm_state, "fig", "green");
    hm_set(keys, vals, hm_state, "grape", "purple");
    hm_set(keys, vals, hm_state, "honeydew", "green");
    hm_set(keys, vals, hm_state, "kiwi", "brown");
    hm_set(keys, vals, hm_state, "lemon", "yellow");
    printf("%s\n", "Length:");
    printf("%d\n", hm_len(hm_state));
    printf("%s\n", "");
    printf("%s\n", "Get tests:");
    _Result_char_ptr _r0 = hm_get(keys, vals, hm_state, "apple");
    if (!_r0._ok) {
        fprintf(stderr, "error: %s\n", _r0._error);
        exit(1);
    }
    char* v1 = _r0._value;
    printf("%s\n", "apple -> {v1}");
    _Result_char_ptr _r1 = hm_get(keys, vals, hm_state, "banana");
    if (!_r1._ok) {
        fprintf(stderr, "error: %s\n", _r1._error);
        exit(1);
    }
    char* v2 = _r1._value;
    printf("%s\n", "banana -> {v2}");
    _Result_char_ptr _r2 = hm_get(keys, vals, hm_state, "grape");
    if (!_r2._ok) {
        fprintf(stderr, "error: %s\n", _r2._error);
        exit(1);
    }
    char* v3 = _r2._value;
    printf("%s\n", "grape -> {v3}");
    _Result_char_ptr _r3 = hm_get(keys, vals, hm_state, "kiwi");
    if (!_r3._ok) {
        fprintf(stderr, "error: %s\n", _r3._error);
        exit(1);
    }
    char* v4 = _r3._value;
    printf("%s\n", "kiwi -> {v4}");
    _Result_char_ptr _r4 = hm_get(keys, vals, hm_state, "lemon");
    if (!_r4._ok) {
        fprintf(stderr, "error: %s\n", _r4._error);
        exit(1);
    }
    char* v5 = _r4._value;
    printf("%s\n", "lemon -> {v5}");
    printf("%s\n", "");
    printf("%s\n", "Has tests:");
    printf("%s\n", "has apple:");
    printf("%d\n", hm_has(keys, hm_state, "apple"));
    printf("%s\n", "has mango:");
    printf("%d\n", hm_has(keys, hm_state, "mango"));
    printf("%s\n", "has cherry:");
    printf("%d\n", hm_has(keys, hm_state, "cherry"));
    printf("%s\n", "has zzz:");
    printf("%d\n", hm_has(keys, hm_state, "zzz"));
    printf("%s\n", "has fig:");
    printf("%d\n", hm_has(keys, hm_state, "fig"));
    printf("%s\n", "");
    printf("%s\n", "Updating 5 entries...");
    hm_set(keys, vals, hm_state, "apple", "green");
    hm_set(keys, vals, hm_state, "banana", "brown");
    hm_set(keys, vals, hm_state, "cherry", "dark red");
    hm_set(keys, vals, hm_state, "grape", "green");
    hm_set(keys, vals, hm_state, "lemon", "bright yellow");
    printf("%s\n", "Length after updates (should be same):");
    printf("%d\n", hm_len(hm_state));
    _Result_char_ptr _r5 = hm_get(keys, vals, hm_state, "apple");
    if (!_r5._ok) {
        fprintf(stderr, "error: %s\n", _r5._error);
        exit(1);
    }
    char* v6 = _r5._value;
    printf("%s\n", "apple -> {v6}");
    _Result_char_ptr _r6 = hm_get(keys, vals, hm_state, "cherry");
    if (!_r6._ok) {
        fprintf(stderr, "error: %s\n", _r6._error);
        exit(1);
    }
    char* v7 = _r6._value;
    printf("%s\n", "cherry -> {v7}");
    printf("%s\n", "");
    printf("%s\n", "Deleting fig, date, elderberry");
    hm_delete(keys, vals, hm_state, "fig");
    hm_delete(keys, vals, hm_state, "date");
    hm_delete(keys, vals, hm_state, "elderberry");
    printf("%s\n", "Length after deletes:");
    printf("%d\n", hm_len(hm_state));
    printf("%s\n", "has fig:");
    printf("%d\n", hm_has(keys, hm_state, "fig"));
    printf("%s\n", "has date:");
    printf("%d\n", hm_has(keys, hm_state, "date"));
    printf("%s\n", "");
    printf("%s\n", "Adding 10 more entries...");
    hm_set(keys, vals, hm_state, "mango", "orange");
    hm_set(keys, vals, hm_state, "nectarine", "orange");
    hm_set(keys, vals, hm_state, "orange", "orange");
    hm_set(keys, vals, hm_state, "papaya", "orange");
    hm_set(keys, vals, hm_state, "quince", "yellow");
    hm_set(keys, vals, hm_state, "raspberry", "red");
    hm_set(keys, vals, hm_state, "strawberry", "red");
    hm_set(keys, vals, hm_state, "tangerine", "orange");
    hm_set(keys, vals, hm_state, "watermelon", "green");
    hm_set(keys, vals, hm_state, "plum", "purple");
    printf("%s\n", "Length:");
    printf("%d\n", hm_len(hm_state));
    printf("%s\n", "");
    printf("%s\n", "Verify after expansion:");
    _Result_char_ptr _r7 = hm_get(keys, vals, hm_state, "apple");
    if (!_r7._ok) {
        fprintf(stderr, "error: %s\n", _r7._error);
        exit(1);
    }
    char* va = _r7._value;
    printf("%s\n", "apple -> {va}");
    _Result_char_ptr _r8 = hm_get(keys, vals, hm_state, "banana");
    if (!_r8._ok) {
        fprintf(stderr, "error: %s\n", _r8._error);
        exit(1);
    }
    char* vb = _r8._value;
    printf("%s\n", "banana -> {vb}");
    _Result_char_ptr _r9 = hm_get(keys, vals, hm_state, "mango");
    if (!_r9._ok) {
        fprintf(stderr, "error: %s\n", _r9._error);
        exit(1);
    }
    char* vm = _r9._value;
    printf("%s\n", "mango -> {vm}");
    _Result_char_ptr _r10 = hm_get(keys, vals, hm_state, "orange");
    if (!_r10._ok) {
        fprintf(stderr, "error: %s\n", _r10._error);
        exit(1);
    }
    char* vo = _r10._value;
    printf("%s\n", "orange -> {vo}");
    _Result_char_ptr _r11 = hm_get(keys, vals, hm_state, "raspberry");
    if (!_r11._ok) {
        fprintf(stderr, "error: %s\n", _r11._error);
        exit(1);
    }
    char* vr = _r11._value;
    printf("%s\n", "raspberry -> {vr}");
    _Result_char_ptr _r12 = hm_get(keys, vals, hm_state, "watermelon");
    if (!_r12._ok) {
        fprintf(stderr, "error: %s\n", _r12._error);
        exit(1);
    }
    char* vw = _r12._value;
    printf("%s\n", "watermelon -> {vw}");
    _Result_char_ptr _r13 = hm_get(keys, vals, hm_state, "plum");
    if (!_r13._ok) {
        fprintf(stderr, "error: %s\n", _r13._error);
        exit(1);
    }
    char* vp = _r13._value;
    printf("%s\n", "plum -> {vp}");
    printf("%s\n", "has strawberry:");
    printf("%d\n", hm_has(keys, hm_state, "strawberry"));
    printf("%s\n", "has fig (deleted):");
    printf("%d\n", hm_has(keys, hm_state, "fig"));
    printf("%s\n", "Final length:");
    printf("%d\n", hm_len(hm_state));
    return 0;
}


