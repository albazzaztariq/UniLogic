#define NULL_PTR 0
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

int list_push(int* values, int* nexts, int head, int next_free, int val);
int list_append(int* values, int* nexts, int head, int next_free, int val);
int list_len(int* nexts, int head);
int list_contains(int* values, int* nexts, int head, int target);
int list_remove(int* values, int* nexts, int head, int target);
void list_print(int* values, int* nexts, int head);
int list_reverse(int* values, int* nexts, int head);
int list_get(int* values, int* nexts, int head, int n);
int list_sum(int* values, int* nexts, int head);
int main(void);

int list_push(int* values, int* nexts, int head, int next_free, int val)
{
    values[next_free] = val;
    nexts[next_free] = head;
    return next_free;
}

int list_append(int* values, int* nexts, int head, int next_free, int val)
{
    values[next_free] = val;
    nexts[next_free] = NULL_PTR;
    if ((head == NULL_PTR)) {
        return next_free;
    }
    int cur = head;
    while ((nexts[cur] != NULL_PTR)) {
        cur = nexts[cur];
    }
    nexts[cur] = next_free;
    return head;
}

int list_len(int* nexts, int head)
{
    int count = 0;
    int cur = head;
    while ((cur != NULL_PTR)) {
        count = (count + 1);
        cur = nexts[cur];
    }
    return count;
}

int list_contains(int* values, int* nexts, int head, int target)
{
    int cur = head;
    while ((cur != NULL_PTR)) {
        if ((values[cur] == target)) {
            return 1;
        }
        cur = nexts[cur];
    }
    return 0;
}

int list_remove(int* values, int* nexts, int head, int target)
{
    if ((head == NULL_PTR)) {
        return NULL_PTR;
    }
    if ((values[head] == target)) {
        return nexts[head];
    }
    int prev = head;
    int cur = nexts[head];
    while ((cur != NULL_PTR)) {
        if ((values[cur] == target)) {
            nexts[prev] = nexts[cur];
            return head;
        }
        prev = cur;
        cur = nexts[cur];
    }
    return head;
}

void list_print(int* values, int* nexts, int head)
{
    int cur = head;
    while ((cur != NULL_PTR)) {
        printf("%d\n", values[cur]);
        cur = nexts[cur];
    }
}

int list_reverse(int* values, int* nexts, int head)
{
    int prev = NULL_PTR;
    int cur = head;
    while ((cur != NULL_PTR)) {
        int next = nexts[cur];
        nexts[cur] = prev;
        prev = cur;
        cur = next;
    }
    return prev;
}

int list_get(int* values, int* nexts, int head, int n)
{
    int cur = head;
    int idx = 0;
    while ((cur != NULL_PTR)) {
        if ((idx == n)) {
            return values[cur];
        }
        idx = (idx + 1);
        cur = nexts[cur];
    }
    return (-1);
}

int list_sum(int* values, int* nexts, int head)
{
    int total = 0;
    int cur = head;
    while ((cur != NULL_PTR)) {
        total = (total + values[cur]);
        cur = nexts[cur];
    }
    return total;
}

int main(void)
{
    int values[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int nexts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int nf = 1;
    int head = NULL_PTR;
    head = list_push(values, nexts, head, nf, 10);
    nf = (nf + 1);
    head = list_push(values, nexts, head, nf, 20);
    nf = (nf + 1);
    head = list_push(values, nexts, head, nf, 30);
    nf = (nf + 1);
    printf("%s\n", "After push 10, 20, 30:");
    list_print(values, nexts, head);
    head = list_append(values, nexts, head, nf, 40);
    nf = (nf + 1);
    head = list_append(values, nexts, head, nf, 50);
    nf = (nf + 1);
    printf("%s\n", "After append 40, 50:");
    list_print(values, nexts, head);
    int len = list_len(nexts, head);
    printf("%s\n", "Length:");
    printf("%d\n", len);
    int has20 = list_contains(values, nexts, head, 20);
    int has99 = list_contains(values, nexts, head, 99);
    printf("%s\n", "Contains 20:");
    printf("%d\n", has20);
    printf("%s\n", "Contains 99:");
    printf("%d\n", has99);
    head = list_remove(values, nexts, head, 20);
    printf("%s\n", "After remove 20:");
    list_print(values, nexts, head);
    head = list_reverse(values, nexts, head);
    printf("%s\n", "After reverse:");
    list_print(values, nexts, head);
    int sum = list_sum(values, nexts, head);
    printf("%s\n", "Sum:");
    printf("%d\n", sum);
    int second = list_get(values, nexts, head, 1);
    printf("%s\n", "Element at index 1:");
    printf("%d\n", second);
    int final_len = list_len(nexts, head);
    printf("%s\n", "Final length:");
    printf("%d\n", final_len);
    return 0;
}


