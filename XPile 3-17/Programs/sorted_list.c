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
#include <stdlib.h>

typedef struct {
    int _ok;
    int _value;
    char* _error;
} _Result_int;

int bisect_arr(int* arr, int lo, int hi, int val);
int bisect_arr_r(int* arr, int lo, int hi, int val);
void shift_up(int* data, int pos, int end_idx);
void shift_down(int* data, int pos, int end_idx);
void shift_data_up(int* data, int from_pos, int total);
int sl_insort(int* data, int start, int len, int val);
void sl_expand(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int pos);
void sl_add(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int val);
_Result_int sl_remove(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int val);
int sl_contains(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int val);
int sl_get(int* data, int* sub_start, int* sub_len, int* state, int idx);
int sl_len(int* state);
void sl_print(int* data, int* state);
int main(void);

int bisect_arr(int* arr, int lo, int hi, int val)
{
    while ((lo < hi)) {
        int mid = ((lo + hi) / 2);
        if ((arr[mid] < val)) {
            lo = (mid + 1);
        } else {
            hi = mid;
        }
    }
    return lo;
}

int bisect_arr_r(int* arr, int lo, int hi, int val)
{
    while ((lo < hi)) {
        int mid = ((lo + hi) / 2);
        if ((arr[mid] <= val)) {
            lo = (mid + 1);
        } else {
            hi = mid;
        }
    }
    return lo;
}

void shift_up(int* data, int pos, int end_idx)
{
    int i = end_idx;
    while ((i > pos)) {
        data[i] = data[(i - 1)];
        i = (i - 1);
    }
}

void shift_down(int* data, int pos, int end_idx)
{
    int i = pos;
    while ((i < (end_idx - 1))) {
        data[i] = data[(i + 1)];
        i = (i + 1);
    }
}

void shift_data_up(int* data, int from_pos, int total)
{
    int i = total;
    while ((i > from_pos)) {
        data[i] = data[(i - 1)];
        i = (i - 1);
    }
}

int sl_insort(int* data, int start, int len, int val)
{
    int pos = bisect_arr(data, start, (start + len), val);
    shift_up(data, pos, (start + len));
    data[pos] = val;
    return pos;
}

void sl_expand(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int pos)
{
    int load = state[2];
    int num_subs = state[0];
    if ((sub_len[pos] > (load * 2))) {
        int old_start = sub_start[pos];
        int split_at = load;
        int i = num_subs;
        while ((i > (pos + 1))) {
            sub_start[i] = sub_start[(i - 1)];
            sub_len[i] = sub_len[(i - 1)];
            sub_max[i] = sub_max[(i - 1)];
            i = (i - 1);
        }
        sub_start[(pos + 1)] = (old_start + split_at);
        sub_len[(pos + 1)] = (sub_len[pos] - split_at);
        sub_max[(pos + 1)] = data[((old_start + sub_len[pos]) - 1)];
        sub_len[pos] = split_at;
        sub_max[pos] = data[((old_start + split_at) - 1)];
        state[0] = (num_subs + 1);
    }
}

void sl_add(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int val)
{
    int num_subs = state[0];
    int total = state[1];
    if ((num_subs == 0)) {
        data[0] = val;
        sub_start[0] = 0;
        sub_len[0] = 1;
        sub_max[0] = val;
        state[0] = 1;
        state[1] = 1;
        return;
    }
    int pos = 0;
    for (int k = 0; k < num_subs; k++) {
        if ((sub_max[k] < val)) {
            pos = (k + 1);
        } else {
            break;
        }
    }
    if ((pos == num_subs)) {
        pos = (num_subs - 1);
        int ins_at = (sub_start[pos] + sub_len[pos]);
        int i = total;
        while ((i > ins_at)) {
            data[i] = data[(i - 1)];
            i = (i - 1);
        }
        data[ins_at] = val;
        sub_len[pos] = (sub_len[pos] + 1);
        sub_max[pos] = val;
        for (int s = (pos + 1); s < num_subs; s++) {
            sub_start[s] = (sub_start[s] + 1);
        }
    } else {
        int s_start = sub_start[pos];
        int s_len = sub_len[pos];
        int ins_pos = bisect_arr(data, s_start, (s_start + s_len), val);
        int i = total;
        while ((i > ins_pos)) {
            data[i] = data[(i - 1)];
            i = (i - 1);
        }
        data[ins_pos] = val;
        sub_len[pos] = (sub_len[pos] + 1);
        if ((val > sub_max[pos])) {
            sub_max[pos] = val;
        }
        for (int s = (pos + 1); s < num_subs; s++) {
            sub_start[s] = (sub_start[s] + 1);
        }
    }
    state[1] = (total + 1);
    sl_expand(data, sub_start, sub_len, sub_max, state, pos);
}

_Result_int sl_remove(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int val)
{
    int num_subs = state[0];
    if ((num_subs == 0)) {
        return (_Result_int){0, 0, "value not in list"};
    }
    int pos = 0;
    for (int k = 0; k < num_subs; k++) {
        if ((sub_max[k] < val)) {
            pos = (k + 1);
        }
    }
    if ((pos == num_subs)) {
        return (_Result_int){0, 0, "value not in list"};
    }
    int s_start = sub_start[pos];
    int s_len = sub_len[pos];
    int idx = bisect_arr(data, s_start, (s_start + s_len), val);
    if ((idx >= (s_start + s_len))) {
        return (_Result_int){0, 0, "value not in list"};
    }
    if ((data[idx] != val)) {
        return (_Result_int){0, 0, "value not in list"};
    }
    int total = state[1];
    int i = idx;
    while ((i < (total - 1))) {
        data[i] = data[(i + 1)];
        i = (i + 1);
    }
    sub_len[pos] = (sub_len[pos] - 1);
    state[1] = (total - 1);
    for (int s = (pos + 1); s < num_subs; s++) {
        sub_start[s] = (sub_start[s] - 1);
    }
    if ((sub_len[pos] == 0)) {
        int j = pos;
        while ((j < (num_subs - 1))) {
            sub_start[j] = sub_start[(j + 1)];
            sub_len[j] = sub_len[(j + 1)];
            sub_max[j] = sub_max[(j + 1)];
            j = (j + 1);
        }
        state[0] = (num_subs - 1);
    } else {
        sub_max[pos] = data[((sub_start[pos] + sub_len[pos]) - 1)];
    }
    return (_Result_int){1, 1, ""};
}

int sl_contains(int* data, int* sub_start, int* sub_len, int* sub_max, int* state, int val)
{
    int num_subs = state[0];
    if ((num_subs == 0)) {
        return 0;
    }
    int pos = 0;
    for (int k = 0; k < num_subs; k++) {
        if ((sub_max[k] < val)) {
            pos = (k + 1);
        }
    }
    if ((pos == num_subs)) {
        return 0;
    }
    int s_start = sub_start[pos];
    int s_len = sub_len[pos];
    int idx = bisect_arr(data, s_start, (s_start + s_len), val);
    if ((idx >= (s_start + s_len))) {
        return 0;
    }
    return (data[idx] == val);
}

int sl_get(int* data, int* sub_start, int* sub_len, int* state, int idx)
{
    int remaining = idx;
    int num_subs = state[0];
    for (int s = 0; s < num_subs; s++) {
        if ((remaining < sub_len[s])) {
            return data[(sub_start[s] + remaining)];
        }
        remaining = (remaining - sub_len[s]);
    }
    return 0;
}

int sl_len(int* state)
{
    return state[1];
}

void sl_print(int* data, int* state)
{
    int total = state[1];
    char* result = "[";
    for (int i = 0; i < total; i++) {
        if ((i > 0)) {
            result = __ul_strcat(result, ", ");
        }
        result = __ul_strcat(result, "{data[i]}");
    }
    result = __ul_strcat(result, "]");
    printf("%s\n", result);
}

int main(void)
{
    int data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int sub_start[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int sub_len[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int sub_max[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int state[] = {0, 0, 8};
    printf("%s\n", "Adding: 50, 20, 80, 10, 40, 90, 30, 70, 60, 15");
    sl_add(data, sub_start, sub_len, sub_max, state, 50);
    sl_add(data, sub_start, sub_len, sub_max, state, 20);
    sl_add(data, sub_start, sub_len, sub_max, state, 80);
    sl_add(data, sub_start, sub_len, sub_max, state, 10);
    sl_add(data, sub_start, sub_len, sub_max, state, 40);
    sl_add(data, sub_start, sub_len, sub_max, state, 90);
    sl_add(data, sub_start, sub_len, sub_max, state, 30);
    sl_add(data, sub_start, sub_len, sub_max, state, 70);
    sl_add(data, sub_start, sub_len, sub_max, state, 60);
    sl_add(data, sub_start, sub_len, sub_max, state, 15);
    printf("%s\n", "After 10 adds:");
    sl_print(data, state);
    printf("%s\n", "Length:");
    printf("%d\n", sl_len(state));
    printf("%s\n", "Contains 40:");
    printf("%d\n", sl_contains(data, sub_start, sub_len, sub_max, state, 40));
    printf("%s\n", "Contains 99:");
    printf("%d\n", sl_contains(data, sub_start, sub_len, sub_max, state, 99));
    printf("%s\n", "Contains 10:");
    printf("%d\n", sl_contains(data, sub_start, sub_len, sub_max, state, 10));
    printf("%s\n", "Contains 90:");
    printf("%d\n", sl_contains(data, sub_start, sub_len, sub_max, state, 90));
    printf("%s\n", "Element at index 0:");
    printf("%d\n", sl_get(data, sub_start, sub_len, state, 0));
    printf("%s\n", "Element at index 5:");
    printf("%d\n", sl_get(data, sub_start, sub_len, state, 5));
    printf("%s\n", "Element at index 9:");
    printf("%d\n", sl_get(data, sub_start, sub_len, state, 9));
    printf("%s\n", "Removing 20, 80, 15");
    _Result_int _r0 = sl_remove(data, sub_start, sub_len, sub_max, state, 20);
    if (!_r0._ok) {
        fprintf(stderr, "error: %s\n", _r0._error);
        exit(1);
    }
    _r0._value;
    _Result_int _r1 = sl_remove(data, sub_start, sub_len, sub_max, state, 80);
    if (!_r1._ok) {
        fprintf(stderr, "error: %s\n", _r1._error);
        exit(1);
    }
    _r1._value;
    _Result_int _r2 = sl_remove(data, sub_start, sub_len, sub_max, state, 15);
    if (!_r2._ok) {
        fprintf(stderr, "error: %s\n", _r2._error);
        exit(1);
    }
    _r2._value;
    printf("%s\n", "After removes:");
    sl_print(data, state);
    printf("%s\n", "Length:");
    printf("%d\n", sl_len(state));
    printf("%s\n", "Adding: 25, 25, 55, 5, 95, 42");
    sl_add(data, sub_start, sub_len, sub_max, state, 25);
    sl_add(data, sub_start, sub_len, sub_max, state, 25);
    sl_add(data, sub_start, sub_len, sub_max, state, 55);
    sl_add(data, sub_start, sub_len, sub_max, state, 5);
    sl_add(data, sub_start, sub_len, sub_max, state, 95);
    sl_add(data, sub_start, sub_len, sub_max, state, 42);
    printf("%s\n", "Final state:");
    sl_print(data, state);
    printf("%s\n", "Length:");
    printf("%d\n", sl_len(state));
    printf("%s\n", "Elements by index:");
    int total = sl_len(state);
    for (int i = 0; i < total; i++) {
        printf("%d\n", sl_get(data, sub_start, sub_len, state, i));
    }
    return 0;
}


