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
static void __ul_print_float(double v) {
    char _buf[64]; snprintf(_buf, sizeof(_buf), "%.10g", v);
    int _has_dot = 0; for (int _i = 0; _buf[_i]; _i++) if (_buf[_i] == '.' || _buf[_i] == 'e') { _has_dot = 1; break; }
    if (!_has_dot) { int _l = (int)strlen(_buf); _buf[_l] = '.'; _buf[_l+1] = '0'; _buf[_l+2] = 0; }
    printf("%s\n", _buf); }

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct {
    int ax;
    int ay;
    int bx;
    int by;
    int cx;
    int cy;
} Triangle;

typedef struct {
    int data[32];
    int top;
} Stack;

typedef struct {
    int data[16];
    int head;
    int tail;
    int count;
} Queue;

typedef struct {
    int val;
    int next;
} ListNode;

typedef struct {
    ListNode pool[64];
    int free_head;
    int list_head;
    int size_c;
} LinkedList;

typedef struct {
    int keys[31];
    int vals[31];
    int used[31];
} HashTable;

int gcd(int a, int b);
int lcm(int a, int b);
int is_prime(int n);
int sieve_primes(int limit, int* primes, int max_out);
int int_sqrt(int n);
int fast_pow(int base, int exp);
void bubble_sort(int* arr, int n);
void insertion_sort(int* arr, int n);
void selection_sort(int* arr, int n);
void counting_sort(int* arr, int n, int max_val);
void merge(int* arr, int lo, int mid, int hi, int* tmp);
void merge_sort_r(int* arr, int lo, int hi, int* tmp);
void merge_sort(int* arr, int n);
int linear_search(int* arr, int n, int target);
int binary_search(int* arr, int n, int target);
int find_min(int* arr, int n);
int find_max(int* arr, int n);
int my_strlen(char* s);
int is_palindrome(char* s, int len);
int count_digits(char* s);
int count_alpha(char* s);
int count_spaces(char* s);
int count_char(char* s, int c);
int parse_uint(char* s);
int str_find(char* h, char* n);
int str_count(char* h, char* n);
int point_dist_sq(Point a, Point b);
int rect_area(Rect r);
int rect_perimeter(Rect r);
int rect_contains(Rect r, Point p);
int triangle_area2(Triangle t);
int popcount(uint32_t x);
uint32_t reverse_bits(uint32_t x);
uint32_t next_pow2(uint32_t n);
int bit_count_range(uint32_t lo, uint32_t hi);
void stack_init(Stack* s);
int stack_empty(Stack* s);
int stack_full(Stack* s);
int stack_push(Stack* s, int v);
int stack_pop(Stack* s);
int stack_peek(Stack* s);
int stack_size(Stack* s);
void queue_init(Queue* q);
int queue_empty(Queue* q);
int queue_full(Queue* q);
int queue_enqueue(Queue* q, int v);
int queue_dequeue(Queue* q);
int queue_front(Queue* q);
void list_init(LinkedList* l);
int list_alloc_node(LinkedList* l);
void list_free_node(LinkedList* l, int idx);
int list_insert_front(LinkedList* l, int val);
int list_search(LinkedList* l, int val);
int list_delete(LinkedList* l, int val);
int list_sum(LinkedList* l);
void ht_init(HashTable* h);
int ht_hash(int key);
int ht_set(HashTable* h, int key, int val);
int ht_get(HashTable* h, int key, int* out);
int fib(int n);
int factorial(int n);
int hanoi_moves(int n);
int bin_coeff(int n, int k);
int gcd_r(int a, int b);
int sum_of_digits(int n);
int digit_count(int n);
int reverse_num(int n);
int is_perfect(int n);
int is_armstrong(int n);
int arr_sum(int* arr, int n);
int arr_mean_x10(int* arr, int n);
int arr_range(int* arr, int n);
int arr_variance_approx(int* arr, int n);
int load_str(char* src, int* dst, int max);
void ia_caesar_encode(int* arr, int len, int shift);
void ia_caesar_decode(int* arr, int len, int shift);
int ia_sum(int* arr, int len);
void ia_reverse(int* arr, int len);
int ia_is_palindrome(int* arr, int len);
int rle_encode(int* src, int slen, int* counts, int* chars, int max_pairs);
int main(void);

int gcd(int a, int b)
{
    while ((b != 0)) {
        int t = b;
        b = (a % b);
        a = t;
    }
    return a;
}

int lcm(int a, int b)
{
    return ((a / gcd(a, b)) * b);
}

int is_prime(int n)
{
    if ((n < 2)) {
        return 0;
    }
    if ((n == 2)) {
        return 1;
    }
    if (((n % 2) == 0)) {
        return 0;
    }
    int i = 3;
    while (((i * i) <= n)) {
        if (((n % i) == 0)) {
            return 0;
        }
        i = (i + 2);
    }
    return 1;
}

int sieve_primes(int limit, int* primes, int max_out)
{
    int sieve[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;
    int j = 0;
    int count = 0;
    if ((limit > 199)) {
        limit = 199;
    }
    i = 0;
    while ((i <= limit)) {
        sieve[i] = 1;
        i = (i + 1);
    }
    sieve[0] = 0;
    sieve[1] = 0;
    i = 2;
    while (((i * i) <= limit)) {
        if (sieve[i]) {
            j = (i * i);
            while ((j <= limit)) {
                sieve[j] = 0;
                j = (j + i);
            }
        }
        i = (i + 1);
    }
    count = 0;
    i = 2;
    while (((i <= limit) && (count < max_out))) {
        if (sieve[i]) {
            primes[count] = i;
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

int int_sqrt(int n)
{
    if ((n <= 0)) {
        return 0;
    }
    int x = n;
    int y = ((x + 1) / 2);
    while ((y < x)) {
        x = y;
        y = ((x + (n / x)) / 2);
    }
    return x;
}

int fast_pow(int base, int exp)
{
    int result = 1;
    while ((exp > 0)) {
        if ((exp & 1)) {
            result = (result * base);
        }
        base = (base * base);
        exp = (exp >> 1);
    }
    return result;
}

void bubble_sort(int* arr, int n)
{
    int i = 0;
    int j = 0;
    int tmp = 0;
    i = 0;
    while ((i < (n - 1))) {
        j = 0;
        while ((j < ((n - 1) - i))) {
            if ((arr[j] > arr[(j + 1)])) {
                tmp = arr[j];
                arr[j] = arr[(j + 1)];
                arr[(j + 1)] = tmp;
            }
            j = (j + 1);
        }
        i = (i + 1);
    }
}

void insertion_sort(int* arr, int n)
{
    int i = 0;
    int j = 0;
    int key = 0;
    i = 1;
    while ((i < n)) {
        key = arr[i];
        j = (i - 1);
        while (((j >= 0) && (arr[j] > key))) {
            arr[(j + 1)] = arr[j];
            j = (j - 1);
        }
        arr[(j + 1)] = key;
        i = (i + 1);
    }
}

void selection_sort(int* arr, int n)
{
    int i = 0;
    int j = 0;
    int min_idx = 0;
    int tmp = 0;
    i = 0;
    while ((i < (n - 1))) {
        min_idx = i;
        j = (i + 1);
        while ((j < n)) {
            if ((arr[j] < arr[min_idx])) {
                min_idx = j;
            }
            j = (j + 1);
        }
        tmp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = tmp;
        i = (i + 1);
    }
}

void counting_sort(int* arr, int n, int max_val)
{
    int count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;
    int j = 0;
    int idx = 0;
    if ((max_val > 127)) {
        max_val = 127;
    }
    i = 0;
    while ((i <= max_val)) {
        count[i] = 0;
        i = (i + 1);
    }
    i = 0;
    while ((i < n)) {
        if (((arr[i] >= 0) && (arr[i] <= max_val))) {
            count[arr[i]] = (count[arr[i]] + 1);
        }
        i = (i + 1);
    }
    idx = 0;
    i = 0;
    while ((i <= max_val)) {
        j = 0;
        while ((j < count[i])) {
            arr[idx] = i;
            idx = (idx + 1);
            j = (j + 1);
        }
        i = (i + 1);
    }
}

void merge(int* arr, int lo, int mid, int hi, int* tmp)
{
    int i = lo;
    int j = (mid + 1);
    int k = lo;
    while (((i <= mid) && (j <= hi))) {
        if ((arr[i] <= arr[j])) {
            tmp[k] = arr[i];
            i = (i + 1);
        } else {
            tmp[k] = arr[j];
            j = (j + 1);
        }
        k = (k + 1);
    }
    while ((i <= mid)) {
        tmp[k] = arr[i];
        i = (i + 1);
        k = (k + 1);
    }
    while ((j <= hi)) {
        tmp[k] = arr[j];
        j = (j + 1);
        k = (k + 1);
    }
    i = lo;
    while ((i <= hi)) {
        arr[i] = tmp[i];
        i = (i + 1);
    }
}

void merge_sort_r(int* arr, int lo, int hi, int* tmp)
{
    if ((lo >= hi)) {
        return;
    }
    int mid = (lo + ((hi - lo) / 2));
    merge_sort_r(arr, lo, mid, tmp);
    merge_sort_r(arr, (mid + 1), hi, tmp);
    merge(arr, lo, mid, hi, tmp);
}

void merge_sort(int* arr, int n)
{
    int tmp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if ((n > 128)) {
        n = 128;
    }
    merge_sort_r(arr, 0, (n - 1), tmp);
}

int linear_search(int* arr, int n, int target)
{
    int i = 0;
    i = 0;
    while ((i < n)) {
        if ((arr[i] == target)) {
            return i;
        }
        i = (i + 1);
    }
    return (-1);
}

int binary_search(int* arr, int n, int target)
{
    int lo = 0;
    int hi = (n - 1);
    int mid = 0;
    while ((lo <= hi)) {
        mid = (lo + ((hi - lo) / 2));
        if ((arr[mid] == target)) {
            return mid;
        }
        if ((arr[mid] < target)) {
            lo = (mid + 1);
        } else {
            hi = (mid - 1);
        }
    }
    return (-1);
}

int find_min(int* arr, int n)
{
    int m = arr[0];
    int i = 0;
    i = 1;
    while ((i < n)) {
        if ((arr[i] < m)) {
            m = arr[i];
        }
        i = (i + 1);
    }
    return m;
}

int find_max(int* arr, int n)
{
    int m = arr[0];
    int i = 0;
    i = 1;
    while ((i < n)) {
        if ((arr[i] > m)) {
            m = arr[i];
        }
        i = (i + 1);
    }
    return m;
}

int my_strlen(char* s)
{
    int n = 0;
    while ((s[n] != 0)) {
        n = (n + 1);
    }
    return n;
}

int is_palindrome(char* s, int len)
{
    int lo = 0;
    int hi = (len - 1);
    while ((lo < hi)) {
        if ((((unsigned char)(s[lo])) != ((unsigned char)(s[hi])))) {
            return 0;
        }
        lo = (lo + 1);
        hi = (hi - 1);
    }
    return 1;
}

int count_digits(char* s)
{
    int n = 0;
    int i = 0;
    while ((s[i] != 0)) {
        int c = ((unsigned char)(s[i]));
        if (((c >= 48) && (c <= 57))) {
            n = (n + 1);
        }
        i = (i + 1);
    }
    return n;
}

int count_alpha(char* s)
{
    int n = 0;
    int i = 0;
    while ((s[i] != 0)) {
        int c = ((unsigned char)(s[i]));
        if ((((c >= 65) && (c <= 90)) || ((c >= 97) && (c <= 122)))) {
            n = (n + 1);
        }
        i = (i + 1);
    }
    return n;
}

int count_spaces(char* s)
{
    int n = 0;
    int i = 0;
    while ((s[i] != 0)) {
        if ((((unsigned char)(s[i])) == 32)) {
            n = (n + 1);
        }
        i = (i + 1);
    }
    return n;
}

int count_char(char* s, int c)
{
    int n = 0;
    int i = 0;
    while ((s[i] != 0)) {
        if ((((unsigned char)(s[i])) == c)) {
            n = (n + 1);
        }
        i = (i + 1);
    }
    return n;
}

int parse_uint(char* s)
{
    int result = 0;
    int i = 0;
    while ((s[i] != 0)) {
        int c = ((unsigned char)(s[i]));
        if (((c >= 48) && (c <= 57))) {
            result = ((result * 10) + (c - 48));
        }
        i = (i + 1);
    }
    return result;
}

int str_find(char* h, char* n)
{
    int hlen = my_strlen(h);
    int nlen = my_strlen(n);
    int i = 0;
    int j = 0;
    int ok_c = 0;
    i = 0;
    while ((i <= (hlen - nlen))) {
        ok_c = 1;
        int _brk_0 = 0;
        j = 0;
        while (((j < nlen) && (!_brk_0))) {
            if ((((unsigned char)(h[(i + j)])) != ((unsigned char)(n[j])))) {
                ok_c = 0;
                _brk_0 = 1;
            }
            if ((!_brk_0)) {
                j = (j + 1);
            }
        }
        if (ok_c) {
            return i;
        }
        i = (i + 1);
    }
    return (-1);
}

int str_count(char* h, char* n)
{
    int hlen = my_strlen(h);
    int nlen = my_strlen(n);
    int count = 0;
    int i = 0;
    int j = 0;
    int ok_c = 0;
    if ((nlen == 0)) {
        return 0;
    }
    while ((i <= (hlen - nlen))) {
        ok_c = 1;
        int _brk_1 = 0;
        j = 0;
        while (((j < nlen) && (!_brk_1))) {
            if ((((unsigned char)(h[(i + j)])) != ((unsigned char)(n[j])))) {
                ok_c = 0;
                _brk_1 = 1;
            }
            if ((!_brk_1)) {
                j = (j + 1);
            }
        }
        if (ok_c) {
            count = (count + 1);
            i = (i + nlen);
        } else {
            i = (i + 1);
        }
    }
    return count;
}

int point_dist_sq(Point a, Point b)
{
    int dx = (a.x - b.x);
    int dy = (a.y - b.y);
    return ((dx * dx) + (dy * dy));
}

int rect_area(Rect r)
{
    return (r.w * r.h);
}

int rect_perimeter(Rect r)
{
    return (2 * (r.w + r.h));
}

int rect_contains(Rect r, Point p)
{
    return ((((p.x >= r.x) && (p.x <= (r.x + r.w))) && (p.y >= r.y)) && (p.y <= (r.y + r.h)));
}

int triangle_area2(Triangle t)
{
    int cross = (((t.bx - t.ax) * (t.cy - t.ay)) - ((t.cx - t.ax) * (t.by - t.ay)));
    if ((cross < 0)) {
        cross = (-cross);
    }
    return cross;
}

int popcount(uint32_t x)
{
    int count = 0;
    while ((x != 0)) {
        count = (count + ((int)(x & 1)));
        x = (x >> 1);
    }
    return count;
}

uint32_t reverse_bits(uint32_t x)
{
    uint32_t result = 0;
    int i = 0;
    i = 0;
    while ((i < 32)) {
        result = ((result << 1) | (x & 1));
        x = (x >> 1);
        i = (i + 1);
    }
    return result;
}

uint32_t next_pow2(uint32_t n)
{
    if ((n == 0)) {
        return 1;
    }
    n = (n - 1);
    n = (n | (n >> 1));
    n = (n | (n >> 2));
    n = (n | (n >> 4));
    n = (n | (n >> 8));
    n = (n | (n >> 16));
    return (n + 1);
}

int bit_count_range(uint32_t lo, uint32_t hi)
{
    int total = 0;
    uint32_t i = lo;
    while ((i <= hi)) {
        total = (total + popcount(i));
        i = (i + 1);
    }
    return total;
}

void stack_init(Stack* s)
{
    s->top = (-1);
}

int stack_empty(Stack* s)
{
    return (s->top < 0);
}

int stack_full(Stack* s)
{
    return (s->top >= (32 - 1));
}

int stack_push(Stack* s, int v)
{
    if (stack_full(s)) {
        return 0;
    }
    s->top = (s->top + 1);
    s->data[s->top] = v;
    return 1;
}

int stack_pop(Stack* s)
{
    if (stack_empty(s)) {
        return (-1);
    }
    int v = s->data[s->top];
    s->top = (s->top - 1);
    return v;
}

int stack_peek(Stack* s)
{
    if (stack_empty(s)) {
        return (-1);
    }
    return s->data[s->top];
}

int stack_size(Stack* s)
{
    return (s->top + 1);
}

void queue_init(Queue* q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

int queue_empty(Queue* q)
{
    return (q->count == 0);
}

int queue_full(Queue* q)
{
    return (q->count == 16);
}

int queue_enqueue(Queue* q, int v)
{
    if (queue_full(q)) {
        return 0;
    }
    q->data[q->tail] = v;
    q->tail = ((q->tail + 1) % 16);
    q->count = (q->count + 1);
    return 1;
}

int queue_dequeue(Queue* q)
{
    if (queue_empty(q)) {
        return (-1);
    }
    int v = q->data[q->head];
    q->head = ((q->head + 1) % 16);
    q->count = (q->count - 1);
    return v;
}

int queue_front(Queue* q)
{
    if (queue_empty(q)) {
        return (-1);
    }
    return q->data[q->head];
}

void list_init(LinkedList* l)
{
    int i = 0;
    i = 0;
    while ((i < (64 - 1))) {
        l->pool[i].next = (i + 1);
        i = (i + 1);
    }
    l->pool[(64 - 1)].next = (-1);
    l->free_head = 0;
    l->list_head = (-1);
    l->size_c = 0;
}

int list_alloc_node(LinkedList* l)
{
    if ((l->free_head == (-1))) {
        return (-1);
    }
    int idx = l->free_head;
    l->free_head = l->pool[idx].next;
    return idx;
}

void list_free_node(LinkedList* l, int idx)
{
    l->pool[idx].next = l->free_head;
    l->free_head = idx;
}

int list_insert_front(LinkedList* l, int val)
{
    int idx = list_alloc_node(l);
    if ((idx == (-1))) {
        return 0;
    }
    l->pool[idx].val = val;
    l->pool[idx].next = l->list_head;
    l->list_head = idx;
    l->size_c = (l->size_c + 1);
    return 1;
}

int list_search(LinkedList* l, int val)
{
    int cur = l->list_head;
    while ((cur != (-1))) {
        if ((l->pool[cur].val == val)) {
            return 1;
        }
        cur = l->pool[cur].next;
    }
    return 0;
}

int list_delete(LinkedList* l, int val)
{
    int cur = l->list_head;
    int prev = (-1);
    while ((cur != (-1))) {
        if ((l->pool[cur].val == val)) {
            if ((prev == (-1))) {
                l->list_head = l->pool[cur].next;
            } else {
                l->pool[prev].next = l->pool[cur].next;
            }
            list_free_node(l, cur);
            l->size_c = (l->size_c - 1);
            return 1;
        }
        prev = cur;
        cur = l->pool[cur].next;
    }
    return 0;
}

int list_sum(LinkedList* l)
{
    int sum = 0;
    int cur = l->list_head;
    while ((cur != (-1))) {
        sum = (sum + l->pool[cur].val);
        cur = l->pool[cur].next;
    }
    return sum;
}

void ht_init(HashTable* h)
{
    int i = 0;
    i = 0;
    while ((i < 31)) {
        h->keys[i] = 0;
        h->vals[i] = 0;
        h->used[i] = 0;
        i = (i + 1);
    }
}

int ht_hash(int key)
{
    int h = (key % 31);
    if ((h < 0)) {
        h = (h + 31);
    }
    return h;
}

int ht_set(HashTable* h, int key, int val)
{
    int i = 0;
    int idx = ht_hash(key);
    i = 0;
    while ((i < 31)) {
        int slot = ((idx + i) % 31);
        if (((!h->used[slot]) || (h->keys[slot] == key))) {
            h->keys[slot] = key;
            h->vals[slot] = val;
            h->used[slot] = 1;
            return 1;
        }
        i = (i + 1);
    }
    return 0;
}

int ht_get(HashTable* h, int key, int* out)
{
    int i = 0;
    int idx = ht_hash(key);
    i = 0;
    while ((i < 31)) {
        int slot = ((idx + i) % 31);
        if ((!h->used[slot])) {
            return 0;
        }
        if ((h->keys[slot] == key)) {
            (*out) = h->vals[slot];
            return 1;
        }
        i = (i + 1);
    }
    return 0;
}

int fib(int n)
{
    if ((n <= 1)) {
        return n;
    }
    return (fib((n - 1)) + fib((n - 2)));
}

int factorial(int n)
{
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

int hanoi_moves(int n)
{
    if ((n == 0)) {
        return 0;
    }
    return ((2 * hanoi_moves((n - 1))) + 1);
}

int bin_coeff(int n, int k)
{
    if (((k == 0) || (k == n))) {
        return 1;
    }
    if ((k > (n - k))) {
        k = (n - k);
    }
    int result = 1;
    int i = 0;
    i = 0;
    while ((i < k)) {
        result = ((result * (n - i)) / (i + 1));
        i = (i + 1);
    }
    return result;
}

int gcd_r(int a, int b)
{
    if ((b == 0)) {
        return a;
    }
    return gcd_r(b, (a % b));
}

int sum_of_digits(int n)
{
    if ((n < 0)) {
        n = (-n);
    }
    int s = 0;
    if ((n == 0)) {
        return 0;
    }
    while ((n > 0)) {
        s = (s + (n % 10));
        n = (n / 10);
    }
    return s;
}

int digit_count(int n)
{
    if ((n == 0)) {
        return 1;
    }
    if ((n < 0)) {
        n = (-n);
    }
    int c = 0;
    while ((n > 0)) {
        c = (c + 1);
        n = (n / 10);
    }
    return c;
}

int reverse_num(int n)
{
    int neg = 0;
    int r = 0;
    if ((n < 0)) {
        neg = 1;
        n = (-n);
    }
    while ((n > 0)) {
        r = ((r * 10) + (n % 10));
        n = (n / 10);
    }
    __auto_type _tern_1 = r;
    if (neg) {
        _tern_1 = (-r);
    }
    return _tern_1;
}

int is_perfect(int n)
{
    if ((n < 2)) {
        return 0;
    }
    int sum = 1;
    int i = 0;
    i = 2;
    while (((i * i) <= n)) {
        if (((n % i) == 0)) {
            sum = (sum + i);
            if ((i != (n / i))) {
                sum = (sum + (n / i));
            }
        }
        i = (i + 1);
    }
    return (sum == n);
}

int is_armstrong(int n)
{
    int orig = n;
    int dc = digit_count(n);
    int s = 0;
    int d = 0;
    while ((n > 0)) {
        d = (n % 10);
        s = (s + fast_pow(d, dc));
        n = (n / 10);
    }
    return (s == orig);
}

int arr_sum(int* arr, int n)
{
    int s = 0;
    int i = 0;
    i = 0;
    while ((i < n)) {
        s = (s + arr[i]);
        i = (i + 1);
    }
    return s;
}

int arr_mean_x10(int* arr, int n)
{
    if ((n == 0)) {
        return 0;
    }
    return ((arr_sum(arr, n) * 10) / n);
}

int arr_range(int* arr, int n)
{
    return (find_max(arr, n) - find_min(arr, n));
}

int arr_variance_approx(int* arr, int n)
{
    if ((n == 0)) {
        return 0;
    }
    int mean = (arr_sum(arr, n) / n);
    int sv = 0;
    int i = 0;
    int d = 0;
    i = 0;
    while ((i < n)) {
        d = (arr[i] - mean);
        sv = (sv + (d * d));
        i = (i + 1);
    }
    return (sv / n);
}

int load_str(char* src, int* dst, int max)
{
    int i = 0;
    while (((src[i] != 0) && (i < (max - 1)))) {
        dst[i] = ((unsigned char)(src[i]));
        i = (i + 1);
    }
    dst[i] = 0;
    return i;
}

void ia_caesar_encode(int* arr, int len, int shift)
{
    int i = 0;
    shift = (((shift % 26) + 26) % 26);
    i = 0;
    while ((i < len)) {
        int c = arr[i];
        if (((c >= 65) && (c <= 90))) {
            arr[i] = (65 + (((c - 65) + shift) % 26));
        } else {
            if (((c >= 97) && (c <= 122))) {
                arr[i] = (97 + (((c - 97) + shift) % 26));
            }
        }
        i = (i + 1);
    }
}

void ia_caesar_decode(int* arr, int len, int shift)
{
    ia_caesar_encode(arr, len, (26 - shift));
}

int ia_sum(int* arr, int len)
{
    int s = 0;
    int i = 0;
    i = 0;
    while ((i < len)) {
        s = (s + arr[i]);
        i = (i + 1);
    }
    return s;
}

void ia_reverse(int* arr, int len)
{
    int lo = 0;
    int hi = (len - 1);
    int tmp = 0;
    while ((lo < hi)) {
        tmp = arr[lo];
        arr[lo] = arr[hi];
        arr[hi] = tmp;
        lo = (lo + 1);
        hi = (hi - 1);
    }
}

int ia_is_palindrome(int* arr, int len)
{
    int lo = 0;
    int hi = (len - 1);
    while ((lo < hi)) {
        if ((arr[lo] != arr[hi])) {
            return 0;
        }
        lo = (lo + 1);
        hi = (hi - 1);
    }
    return 1;
}

int rle_encode(int* src, int slen, int* counts, int* chars, int max_pairs)
{
    int n = 0;
    int i = 0;
    while (((i < slen) && (n < max_pairs))) {
        int cur = src[i];
        int cnt = 1;
        while ((((i + cnt) < slen) && (src[(i + cnt)] == cur))) {
            cnt = (cnt + 1);
        }
        counts[n] = cnt;
        chars[n] = cur;
        n = (n + 1);
        i = (i + cnt);
    }
    return n;
}

int main(void)
{
    int i = 0;
    printf("%s\n", "=== A. MATH ===");
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", gcd(48, 18));
    printf("%s\n", __ul_strcat("gcd(48,18)=", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", gcd(100, 75));
    printf("%s\n", __ul_strcat("gcd(100,75)=", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", lcm(4, 6));
    printf("%s\n", __ul_strcat("lcm(4,6)=", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", lcm(12, 18));
    printf("%s\n", __ul_strcat("lcm(12,18)=", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", is_prime(1));
    printf("%s\n", __ul_strcat("is_prime(1)=", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", is_prime(2));
    printf("%s\n", __ul_strcat("is_prime(2)=", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", is_prime(17));
    printf("%s\n", __ul_strcat("is_prime(17)=", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", is_prime(100));
    printf("%s\n", __ul_strcat("is_prime(100)=", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", is_prime(97));
    printf("%s\n", __ul_strcat("is_prime(97)=", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", int_sqrt(0));
    printf("%s\n", __ul_strcat("int_sqrt(0)=", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", int_sqrt(9));
    printf("%s\n", __ul_strcat("int_sqrt(9)=", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", int_sqrt(10));
    printf("%s\n", __ul_strcat("int_sqrt(10)=", _cast_buf_11));
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", int_sqrt(144));
    printf("%s\n", __ul_strcat("int_sqrt(144)=", _cast_buf_12));
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", fast_pow(2, 10));
    printf("%s\n", __ul_strcat("fast_pow(2,10)=", _cast_buf_13));
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", fast_pow(3, 5));
    printf("%s\n", __ul_strcat("fast_pow(3,5)=", _cast_buf_14));
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", fast_pow(7, 0));
    printf("%s\n", __ul_strcat("fast_pow(7,0)=", _cast_buf_15));
    int primes[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int pc = sieve_primes(100, primes, 25);
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", pc);
    printf("%s\n", __ul_strcat("primes<=100: count=", _cast_buf_16));
    static char _cast_buf_17[64];
    snprintf(_cast_buf_17, sizeof(_cast_buf_17), "%d", primes[0]);
    static char _cast_buf_18[64];
    snprintf(_cast_buf_18, sizeof(_cast_buf_18), "%d", primes[1]);
    static char _cast_buf_19[64];
    snprintf(_cast_buf_19, sizeof(_cast_buf_19), "%d", primes[2]);
    static char _cast_buf_20[64];
    snprintf(_cast_buf_20, sizeof(_cast_buf_20), "%d", primes[3]);
    static char _cast_buf_21[64];
    snprintf(_cast_buf_21, sizeof(_cast_buf_21), "%d", primes[4]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("first5: ", _cast_buf_17), " "), _cast_buf_18), " "), _cast_buf_19), " "), _cast_buf_20), " "), _cast_buf_21));
    static char _cast_buf_22[64];
    snprintf(_cast_buf_22, sizeof(_cast_buf_22), "%d", primes[(pc - 5)]);
    static char _cast_buf_23[64];
    snprintf(_cast_buf_23, sizeof(_cast_buf_23), "%d", primes[(pc - 4)]);
    static char _cast_buf_24[64];
    snprintf(_cast_buf_24, sizeof(_cast_buf_24), "%d", primes[(pc - 3)]);
    static char _cast_buf_25[64];
    snprintf(_cast_buf_25, sizeof(_cast_buf_25), "%d", primes[(pc - 2)]);
    static char _cast_buf_26[64];
    snprintf(_cast_buf_26, sizeof(_cast_buf_26), "%d", primes[(pc - 1)]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("last5: ", _cast_buf_22), " "), _cast_buf_23), " "), _cast_buf_24), " "), _cast_buf_25), " "), _cast_buf_26));
    printf("%s\n", "=== B. SORT ===");
    int a1[] = {0, 0, 0, 0, 0, 0, 0, 0};
    a1[0] = 64;
    a1[1] = 34;
    a1[2] = 25;
    a1[3] = 12;
    a1[4] = 22;
    a1[5] = 11;
    a1[6] = 90;
    a1[7] = 1;
    bubble_sort(a1, 8);
    static char _cast_buf_27[64];
    snprintf(_cast_buf_27, sizeof(_cast_buf_27), "%d", a1[0]);
    static char _cast_buf_28[64];
    snprintf(_cast_buf_28, sizeof(_cast_buf_28), "%d", a1[1]);
    static char _cast_buf_29[64];
    snprintf(_cast_buf_29, sizeof(_cast_buf_29), "%d", a1[2]);
    static char _cast_buf_30[64];
    snprintf(_cast_buf_30, sizeof(_cast_buf_30), "%d", a1[3]);
    static char _cast_buf_31[64];
    snprintf(_cast_buf_31, sizeof(_cast_buf_31), "%d", a1[4]);
    static char _cast_buf_32[64];
    snprintf(_cast_buf_32, sizeof(_cast_buf_32), "%d", a1[5]);
    static char _cast_buf_33[64];
    snprintf(_cast_buf_33, sizeof(_cast_buf_33), "%d", a1[6]);
    static char _cast_buf_34[64];
    snprintf(_cast_buf_34, sizeof(_cast_buf_34), "%d", a1[7]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("bubble: ", _cast_buf_27), " "), _cast_buf_28), " "), _cast_buf_29), " "), _cast_buf_30), " "), _cast_buf_31), " "), _cast_buf_32), " "), _cast_buf_33), " "), _cast_buf_34));
    int a2[] = {0, 0, 0, 0, 0, 0, 0, 0};
    a2[0] = 5;
    a2[1] = 3;
    a2[2] = 8;
    a2[3] = 1;
    a2[4] = 9;
    a2[5] = 2;
    a2[6] = 7;
    a2[7] = 4;
    insertion_sort(a2, 8);
    static char _cast_buf_35[64];
    snprintf(_cast_buf_35, sizeof(_cast_buf_35), "%d", a2[0]);
    static char _cast_buf_36[64];
    snprintf(_cast_buf_36, sizeof(_cast_buf_36), "%d", a2[1]);
    static char _cast_buf_37[64];
    snprintf(_cast_buf_37, sizeof(_cast_buf_37), "%d", a2[2]);
    static char _cast_buf_38[64];
    snprintf(_cast_buf_38, sizeof(_cast_buf_38), "%d", a2[3]);
    static char _cast_buf_39[64];
    snprintf(_cast_buf_39, sizeof(_cast_buf_39), "%d", a2[4]);
    static char _cast_buf_40[64];
    snprintf(_cast_buf_40, sizeof(_cast_buf_40), "%d", a2[5]);
    static char _cast_buf_41[64];
    snprintf(_cast_buf_41, sizeof(_cast_buf_41), "%d", a2[6]);
    static char _cast_buf_42[64];
    snprintf(_cast_buf_42, sizeof(_cast_buf_42), "%d", a2[7]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("insertion: ", _cast_buf_35), " "), _cast_buf_36), " "), _cast_buf_37), " "), _cast_buf_38), " "), _cast_buf_39), " "), _cast_buf_40), " "), _cast_buf_41), " "), _cast_buf_42));
    int a3[] = {0, 0, 0, 0, 0, 0, 0, 0};
    a3[0] = 10;
    a3[1] = 7;
    a3[2] = 3;
    a3[3] = 9;
    a3[4] = 1;
    a3[5] = 5;
    a3[6] = 6;
    a3[7] = 2;
    selection_sort(a3, 8);
    static char _cast_buf_43[64];
    snprintf(_cast_buf_43, sizeof(_cast_buf_43), "%d", a3[0]);
    static char _cast_buf_44[64];
    snprintf(_cast_buf_44, sizeof(_cast_buf_44), "%d", a3[1]);
    static char _cast_buf_45[64];
    snprintf(_cast_buf_45, sizeof(_cast_buf_45), "%d", a3[2]);
    static char _cast_buf_46[64];
    snprintf(_cast_buf_46, sizeof(_cast_buf_46), "%d", a3[3]);
    static char _cast_buf_47[64];
    snprintf(_cast_buf_47, sizeof(_cast_buf_47), "%d", a3[4]);
    static char _cast_buf_48[64];
    snprintf(_cast_buf_48, sizeof(_cast_buf_48), "%d", a3[5]);
    static char _cast_buf_49[64];
    snprintf(_cast_buf_49, sizeof(_cast_buf_49), "%d", a3[6]);
    static char _cast_buf_50[64];
    snprintf(_cast_buf_50, sizeof(_cast_buf_50), "%d", a3[7]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("selection: ", _cast_buf_43), " "), _cast_buf_44), " "), _cast_buf_45), " "), _cast_buf_46), " "), _cast_buf_47), " "), _cast_buf_48), " "), _cast_buf_49), " "), _cast_buf_50));
    int a4[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    a4[0] = 4;
    a4[1] = 2;
    a4[2] = 7;
    a4[3] = 1;
    a4[4] = 5;
    a4[5] = 3;
    a4[6] = 6;
    a4[7] = 0;
    a4[8] = 9;
    a4[9] = 8;
    counting_sort(a4, 10, 9);
    static char _cast_buf_51[64];
    snprintf(_cast_buf_51, sizeof(_cast_buf_51), "%d", a4[0]);
    static char _cast_buf_52[64];
    snprintf(_cast_buf_52, sizeof(_cast_buf_52), "%d", a4[1]);
    static char _cast_buf_53[64];
    snprintf(_cast_buf_53, sizeof(_cast_buf_53), "%d", a4[2]);
    static char _cast_buf_54[64];
    snprintf(_cast_buf_54, sizeof(_cast_buf_54), "%d", a4[3]);
    static char _cast_buf_55[64];
    snprintf(_cast_buf_55, sizeof(_cast_buf_55), "%d", a4[4]);
    static char _cast_buf_56[64];
    snprintf(_cast_buf_56, sizeof(_cast_buf_56), "%d", a4[5]);
    static char _cast_buf_57[64];
    snprintf(_cast_buf_57, sizeof(_cast_buf_57), "%d", a4[6]);
    static char _cast_buf_58[64];
    snprintf(_cast_buf_58, sizeof(_cast_buf_58), "%d", a4[7]);
    static char _cast_buf_59[64];
    snprintf(_cast_buf_59, sizeof(_cast_buf_59), "%d", a4[8]);
    static char _cast_buf_60[64];
    snprintf(_cast_buf_60, sizeof(_cast_buf_60), "%d", a4[9]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("counting: ", _cast_buf_51), " "), _cast_buf_52), " "), _cast_buf_53), " "), _cast_buf_54), " "), _cast_buf_55), " "), _cast_buf_56), " "), _cast_buf_57), " "), _cast_buf_58), " "), _cast_buf_59), " "), _cast_buf_60));
    int a5[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    a5[0] = 38;
    a5[1] = 27;
    a5[2] = 43;
    a5[3] = 3;
    a5[4] = 9;
    a5[5] = 82;
    a5[6] = 10;
    a5[7] = 1;
    a5[8] = 100;
    a5[9] = 55;
    a5[10] = 44;
    a5[11] = 22;
    merge_sort(a5, 12);
    static char _cast_buf_61[64];
    snprintf(_cast_buf_61, sizeof(_cast_buf_61), "%d", a5[0]);
    static char _cast_buf_62[64];
    snprintf(_cast_buf_62, sizeof(_cast_buf_62), "%d", a5[1]);
    static char _cast_buf_63[64];
    snprintf(_cast_buf_63, sizeof(_cast_buf_63), "%d", a5[2]);
    static char _cast_buf_64[64];
    snprintf(_cast_buf_64, sizeof(_cast_buf_64), "%d", a5[3]);
    static char _cast_buf_65[64];
    snprintf(_cast_buf_65, sizeof(_cast_buf_65), "%d", a5[4]);
    static char _cast_buf_66[64];
    snprintf(_cast_buf_66, sizeof(_cast_buf_66), "%d", a5[5]);
    static char _cast_buf_67[64];
    snprintf(_cast_buf_67, sizeof(_cast_buf_67), "%d", a5[6]);
    static char _cast_buf_68[64];
    snprintf(_cast_buf_68, sizeof(_cast_buf_68), "%d", a5[7]);
    static char _cast_buf_69[64];
    snprintf(_cast_buf_69, sizeof(_cast_buf_69), "%d", a5[8]);
    static char _cast_buf_70[64];
    snprintf(_cast_buf_70, sizeof(_cast_buf_70), "%d", a5[9]);
    static char _cast_buf_71[64];
    snprintf(_cast_buf_71, sizeof(_cast_buf_71), "%d", a5[10]);
    static char _cast_buf_72[64];
    snprintf(_cast_buf_72, sizeof(_cast_buf_72), "%d", a5[11]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("merge: ", _cast_buf_61), " "), _cast_buf_62), " "), _cast_buf_63), " "), _cast_buf_64), " "), _cast_buf_65), " "), _cast_buf_66), " "), _cast_buf_67), " "), _cast_buf_68), " "), _cast_buf_69), " "), _cast_buf_70), " "), _cast_buf_71), " "), _cast_buf_72));
    printf("%s\n", "=== C. SEARCH ===");
    int arr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    i = 0;
    while ((i < 10)) {
        arr[i] = ((i * 3) + 1);
        i = (i + 1);
    }
    static char _cast_buf_73[64];
    snprintf(_cast_buf_73, sizeof(_cast_buf_73), "%d", linear_search(arr, 10, 13));
    printf("%s\n", __ul_strcat("linear(13)=", _cast_buf_73));
    static char _cast_buf_74[64];
    snprintf(_cast_buf_74, sizeof(_cast_buf_74), "%d", linear_search(arr, 10, 5));
    printf("%s\n", __ul_strcat("linear(5)=", _cast_buf_74));
    static char _cast_buf_75[64];
    snprintf(_cast_buf_75, sizeof(_cast_buf_75), "%d", binary_search(arr, 10, 19));
    printf("%s\n", __ul_strcat("binary(19)=", _cast_buf_75));
    static char _cast_buf_76[64];
    snprintf(_cast_buf_76, sizeof(_cast_buf_76), "%d", binary_search(arr, 10, 2));
    printf("%s\n", __ul_strcat("binary(2)=", _cast_buf_76));
    static char _cast_buf_77[64];
    snprintf(_cast_buf_77, sizeof(_cast_buf_77), "%d", find_min(arr, 10));
    static char _cast_buf_78[64];
    snprintf(_cast_buf_78, sizeof(_cast_buf_78), "%d", find_max(arr, 10));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("min=", _cast_buf_77), " max="), _cast_buf_78));
    printf("%s\n", "=== D. STRINGS ===");
    char* str_d1 = "Hello World 123!";
    static char _cast_buf_79[64];
    snprintf(_cast_buf_79, sizeof(_cast_buf_79), "%d", my_strlen(str_d1));
    printf("%s\n", __ul_strcat("len=", _cast_buf_79));
    static char _cast_buf_80[64];
    snprintf(_cast_buf_80, sizeof(_cast_buf_80), "%d", count_digits(str_d1));
    printf("%s\n", __ul_strcat("digits=", _cast_buf_80));
    static char _cast_buf_81[64];
    snprintf(_cast_buf_81, sizeof(_cast_buf_81), "%d", count_alpha(str_d1));
    printf("%s\n", __ul_strcat("alpha=", _cast_buf_81));
    static char _cast_buf_82[64];
    snprintf(_cast_buf_82, sizeof(_cast_buf_82), "%d", count_spaces(str_d1));
    printf("%s\n", __ul_strcat("spaces=", _cast_buf_82));
    static char _cast_buf_83[64];
    snprintf(_cast_buf_83, sizeof(_cast_buf_83), "%d", count_char(str_d1, 108));
    printf("%s\n", __ul_strcat("count_l=", _cast_buf_83));
    static char _cast_buf_84[64];
    snprintf(_cast_buf_84, sizeof(_cast_buf_84), "%d", is_palindrome("racecar", 7));
    printf("%s\n", __ul_strcat("palindrome(racecar)=", _cast_buf_84));
    static char _cast_buf_85[64];
    snprintf(_cast_buf_85, sizeof(_cast_buf_85), "%d", is_palindrome("hello", 5));
    printf("%s\n", __ul_strcat("palindrome(hello)=", _cast_buf_85));
    static char _cast_buf_86[64];
    snprintf(_cast_buf_86, sizeof(_cast_buf_86), "%d", is_palindrome("abcba", 5));
    printf("%s\n", __ul_strcat("palindrome(abcba)=", _cast_buf_86));
    static char _cast_buf_87[64];
    snprintf(_cast_buf_87, sizeof(_cast_buf_87), "%d", parse_uint("4291"));
    printf("%s\n", __ul_strcat("parse_uint(4291)=", _cast_buf_87));
    static char _cast_buf_88[64];
    snprintf(_cast_buf_88, sizeof(_cast_buf_88), "%d", parse_uint("0"));
    printf("%s\n", __ul_strcat("parse_uint(0)=", _cast_buf_88));
    static char _cast_buf_89[64];
    snprintf(_cast_buf_89, sizeof(_cast_buf_89), "%d", parse_uint("99999"));
    printf("%s\n", __ul_strcat("parse_uint(99999)=", _cast_buf_89));
    static char _cast_buf_90[64];
    snprintf(_cast_buf_90, sizeof(_cast_buf_90), "%d", str_find("the cat sat", "cat"));
    printf("%s\n", __ul_strcat("str_find(the_cat_sat,cat)=", _cast_buf_90));
    static char _cast_buf_91[64];
    snprintf(_cast_buf_91, sizeof(_cast_buf_91), "%d", str_find("hello", "xyz"));
    printf("%s\n", __ul_strcat("str_find(hello,xyz)=", _cast_buf_91));
    static char _cast_buf_92[64];
    snprintf(_cast_buf_92, sizeof(_cast_buf_92), "%d", str_find("abcabc", "bc"));
    printf("%s\n", __ul_strcat("str_find(abcabc,bc)=", _cast_buf_92));
    static char _cast_buf_93[64];
    snprintf(_cast_buf_93, sizeof(_cast_buf_93), "%d", str_count("banana", "an"));
    printf("%s\n", __ul_strcat("str_count(banana,an)=", _cast_buf_93));
    static char _cast_buf_94[64];
    snprintf(_cast_buf_94, sizeof(_cast_buf_94), "%d", str_count("aaa", "aa"));
    printf("%s\n", __ul_strcat("str_count(aaa,aa)=", _cast_buf_94));
    static char _cast_buf_95[64];
    snprintf(_cast_buf_95, sizeof(_cast_buf_95), "%d", str_count("abcabc", "abc"));
    printf("%s\n", __ul_strcat("str_count(abcabc,abc)=", _cast_buf_95));
    printf("%s\n", "=== E. STRUCTS ===");
    Point a = {0};
    a.x = 3;
    a.y = 4;
    Point b = {0};
    b.x = 0;
    b.y = 0;
    static char _cast_buf_96[64];
    snprintf(_cast_buf_96, sizeof(_cast_buf_96), "%d", point_dist_sq(a, b));
    printf("%s\n", __ul_strcat("dist_sq((3,4),(0,0))=", _cast_buf_96));
    Rect r = {0};
    r.x = 0;
    r.y = 0;
    r.w = 10;
    r.h = 5;
    static char _cast_buf_97[64];
    snprintf(_cast_buf_97, sizeof(_cast_buf_97), "%d", rect_area(r));
    printf("%s\n", __ul_strcat("rect_area=", _cast_buf_97));
    static char _cast_buf_98[64];
    snprintf(_cast_buf_98, sizeof(_cast_buf_98), "%d", rect_perimeter(r));
    printf("%s\n", __ul_strcat("rect_perim=", _cast_buf_98));
    Point p1 = {0};
    p1.x = 5;
    p1.y = 3;
    Point p2 = {0};
    p2.x = 11;
    p2.y = 3;
    static char _cast_buf_99[64];
    snprintf(_cast_buf_99, sizeof(_cast_buf_99), "%d", rect_contains(r, p1));
    printf("%s\n", __ul_strcat("contains(5,3)=", _cast_buf_99));
    static char _cast_buf_100[64];
    snprintf(_cast_buf_100, sizeof(_cast_buf_100), "%d", rect_contains(r, p2));
    printf("%s\n", __ul_strcat("contains(11,3)=", _cast_buf_100));
    Triangle t = {0};
    t.ax = 0;
    t.ay = 0;
    t.bx = 4;
    t.by = 0;
    t.cx = 0;
    t.cy = 3;
    static char _cast_buf_101[64];
    snprintf(_cast_buf_101, sizeof(_cast_buf_101), "%d", triangle_area2(t));
    printf("%s\n", __ul_strcat("tri_area2=", _cast_buf_101));
    printf("%s\n", "=== F. BITS ===");
    static char _cast_buf_102[64];
    snprintf(_cast_buf_102, sizeof(_cast_buf_102), "%d", popcount(0));
    printf("%s\n", __ul_strcat("popcount(0)=", _cast_buf_102));
    static char _cast_buf_103[64];
    snprintf(_cast_buf_103, sizeof(_cast_buf_103), "%d", popcount(255));
    printf("%s\n", __ul_strcat("popcount(255)=", _cast_buf_103));
    static char _cast_buf_104[64];
    snprintf(_cast_buf_104, sizeof(_cast_buf_104), "%d", popcount(65535));
    printf("%s\n", __ul_strcat("popcount(65535)=", _cast_buf_104));
    static char _cast_buf_105[64];
    snprintf(_cast_buf_105, sizeof(_cast_buf_105), "%d", popcount(2147483649LL));
    printf("%s\n", __ul_strcat("popcount(0x80000001)=", _cast_buf_105));
    static char _cast_buf_106[64];
    snprintf(_cast_buf_106, sizeof(_cast_buf_106), "%u", next_pow2(0));
    printf("%s\n", __ul_strcat("next_pow2(0)=", _cast_buf_106));
    static char _cast_buf_107[64];
    snprintf(_cast_buf_107, sizeof(_cast_buf_107), "%u", next_pow2(1));
    printf("%s\n", __ul_strcat("next_pow2(1)=", _cast_buf_107));
    static char _cast_buf_108[64];
    snprintf(_cast_buf_108, sizeof(_cast_buf_108), "%u", next_pow2(5));
    printf("%s\n", __ul_strcat("next_pow2(5)=", _cast_buf_108));
    static char _cast_buf_109[64];
    snprintf(_cast_buf_109, sizeof(_cast_buf_109), "%u", next_pow2(8));
    printf("%s\n", __ul_strcat("next_pow2(8)=", _cast_buf_109));
    static char _cast_buf_110[64];
    snprintf(_cast_buf_110, sizeof(_cast_buf_110), "%u", next_pow2(9));
    printf("%s\n", __ul_strcat("next_pow2(9)=", _cast_buf_110));
    static char _cast_buf_111[64];
    snprintf(_cast_buf_111, sizeof(_cast_buf_111), "%d", bit_count_range(0, 7));
    printf("%s\n", __ul_strcat("bit_range(0,7)=", _cast_buf_111));
    static char _cast_buf_112[64];
    snprintf(_cast_buf_112, sizeof(_cast_buf_112), "%d", bit_count_range(1, 4));
    printf("%s\n", __ul_strcat("bit_range(1,4)=", _cast_buf_112));
    static char _cast_buf_113[64];
    snprintf(_cast_buf_113, sizeof(_cast_buf_113), "%u", reverse_bits(1));
    printf("%s\n", __ul_strcat("rev_bits(1)=", _cast_buf_113));
    static char _cast_buf_114[64];
    snprintf(_cast_buf_114, sizeof(_cast_buf_114), "%u", reverse_bits(2147483648LL));
    printf("%s\n", __ul_strcat("rev_bits(0x80000000)=", _cast_buf_114));
    printf("%s\n", "=== G. STACK ===");
    Stack stk = {0};
    stack_init((&stk));
    static char _cast_buf_115[64];
    snprintf(_cast_buf_115, sizeof(_cast_buf_115), "%d", stack_empty((&stk)));
    printf("%s\n", __ul_strcat("empty=", _cast_buf_115));
    i = 1;
    while ((i <= 5)) {
        stack_push((&stk), (i * 10));
        i = (i + 1);
    }
    static char _cast_buf_116[64];
    snprintf(_cast_buf_116, sizeof(_cast_buf_116), "%d", stack_size((&stk)));
    printf("%s\n", __ul_strcat("size=", _cast_buf_116));
    static char _cast_buf_117[64];
    snprintf(_cast_buf_117, sizeof(_cast_buf_117), "%d", stack_peek((&stk)));
    printf("%s\n", __ul_strcat("peek=", _cast_buf_117));
    while ((!stack_empty((&stk)))) {
        static char _cast_buf_118[64];
        snprintf(_cast_buf_118, sizeof(_cast_buf_118), "%d", stack_pop((&stk)));
        printf("%s\n", __ul_strcat("pop=", _cast_buf_118));
    }
    static char _cast_buf_119[64];
    snprintf(_cast_buf_119, sizeof(_cast_buf_119), "%d", stack_empty((&stk)));
    printf("%s\n", __ul_strcat("empty=", _cast_buf_119));
    printf("%s\n", "=== H. QUEUE ===");
    Queue q = {0};
    queue_init((&q));
    static char _cast_buf_120[64];
    snprintf(_cast_buf_120, sizeof(_cast_buf_120), "%d", queue_empty((&q)));
    printf("%s\n", __ul_strcat("empty=", _cast_buf_120));
    i = 0;
    while ((i < 5)) {
        queue_enqueue((&q), ((i * 7) + 1));
        i = (i + 1);
    }
    static char _cast_buf_121[64];
    snprintf(_cast_buf_121, sizeof(_cast_buf_121), "%d", q.count);
    printf("%s\n", __ul_strcat("count=", _cast_buf_121));
    static char _cast_buf_122[64];
    snprintf(_cast_buf_122, sizeof(_cast_buf_122), "%d", queue_front((&q)));
    printf("%s\n", __ul_strcat("front=", _cast_buf_122));
    while ((!queue_empty((&q)))) {
        static char _cast_buf_123[64];
        snprintf(_cast_buf_123, sizeof(_cast_buf_123), "%d", queue_dequeue((&q)));
        printf("%s\n", __ul_strcat("deq=", _cast_buf_123));
    }
    static char _cast_buf_124[64];
    snprintf(_cast_buf_124, sizeof(_cast_buf_124), "%d", queue_empty((&q)));
    printf("%s\n", __ul_strcat("empty=", _cast_buf_124));
    printf("%s\n", "=== I. LIST ===");
    LinkedList lst = {0};
    list_init((&lst));
    i = 1;
    while ((i <= 6)) {
        list_insert_front((&lst), (i * 5));
        i = (i + 1);
    }
    static char _cast_buf_125[64];
    snprintf(_cast_buf_125, sizeof(_cast_buf_125), "%d", lst.size_c);
    printf("%s\n", __ul_strcat("size=", _cast_buf_125));
    static char _cast_buf_126[64];
    snprintf(_cast_buf_126, sizeof(_cast_buf_126), "%d", list_sum((&lst)));
    printf("%s\n", __ul_strcat("sum=", _cast_buf_126));
    static char _cast_buf_127[64];
    snprintf(_cast_buf_127, sizeof(_cast_buf_127), "%d", list_search((&lst), 15));
    printf("%s\n", __ul_strcat("search(15)=", _cast_buf_127));
    static char _cast_buf_128[64];
    snprintf(_cast_buf_128, sizeof(_cast_buf_128), "%d", list_search((&lst), 99));
    printf("%s\n", __ul_strcat("search(99)=", _cast_buf_128));
    list_delete((&lst), 15);
    static char _cast_buf_129[64];
    snprintf(_cast_buf_129, sizeof(_cast_buf_129), "%d", lst.size_c);
    printf("%s\n", __ul_strcat("after_del size=", _cast_buf_129));
    static char _cast_buf_130[64];
    snprintf(_cast_buf_130, sizeof(_cast_buf_130), "%d", list_sum((&lst)));
    printf("%s\n", __ul_strcat("after_del sum=", _cast_buf_130));
    printf("%s\n", "=== J. HASH ===");
    HashTable ht = {0};
    ht_init((&ht));
    i = 0;
    while ((i < 10)) {
        ht_set((&ht), ((i * 7) + 3), ((i * i) + 1));
        i = (i + 1);
    }
    int out = 0;
    i = 0;
    while ((i < 10)) {
        int found = ht_get((&ht), ((i * 7) + 3), (&out));
        __auto_type _tern_2 = (-1);
        if (found) {
            _tern_2 = out;
        }
        static char _cast_buf_131[64];
        snprintf(_cast_buf_131, sizeof(_cast_buf_131), "%d", ((i * 7) + 3));
        static char _cast_buf_132[64];
        snprintf(_cast_buf_132, sizeof(_cast_buf_132), "%d", _tern_2);
        static char _cast_buf_133[64];
        snprintf(_cast_buf_133, sizeof(_cast_buf_133), "%d", found);
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("ht[", _cast_buf_131), "]="), _cast_buf_132), " found="), _cast_buf_133));
        i = (i + 1);
    }
    static char _cast_buf_134[64];
    snprintf(_cast_buf_134, sizeof(_cast_buf_134), "%d", ht_get((&ht), 999, (&out)));
    printf("%s\n", __ul_strcat("ht[999] found=", _cast_buf_134));
    printf("%s\n", "=== K. RECURSION ===");
    i = 0;
    while ((i <= 10)) {
        static char _cast_buf_135[64];
        snprintf(_cast_buf_135, sizeof(_cast_buf_135), "%d", i);
        static char _cast_buf_136[64];
        snprintf(_cast_buf_136, sizeof(_cast_buf_136), "%d", fib(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("fib(", _cast_buf_135), ")="), _cast_buf_136));
        i = (i + 1);
    }
    i = 0;
    while ((i <= 8)) {
        static char _cast_buf_137[64];
        snprintf(_cast_buf_137, sizeof(_cast_buf_137), "%d", i);
        static char _cast_buf_138[64];
        snprintf(_cast_buf_138, sizeof(_cast_buf_138), "%d", factorial(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("fact(", _cast_buf_137), ")="), _cast_buf_138));
        i = (i + 1);
    }
    i = 0;
    while ((i <= 6)) {
        static char _cast_buf_139[64];
        snprintf(_cast_buf_139, sizeof(_cast_buf_139), "%d", i);
        static char _cast_buf_140[64];
        snprintf(_cast_buf_140, sizeof(_cast_buf_140), "%d", hanoi_moves(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("hanoi(", _cast_buf_139), ")="), _cast_buf_140));
        i = (i + 1);
    }
    static char _cast_buf_141[64];
    snprintf(_cast_buf_141, sizeof(_cast_buf_141), "%d", bin_coeff(5, 2));
    printf("%s\n", __ul_strcat("C(5,2)=", _cast_buf_141));
    static char _cast_buf_142[64];
    snprintf(_cast_buf_142, sizeof(_cast_buf_142), "%d", bin_coeff(10, 3));
    printf("%s\n", __ul_strcat("C(10,3)=", _cast_buf_142));
    static char _cast_buf_143[64];
    snprintf(_cast_buf_143, sizeof(_cast_buf_143), "%d", bin_coeff(6, 6));
    printf("%s\n", __ul_strcat("C(6,6)=", _cast_buf_143));
    static char _cast_buf_144[64];
    snprintf(_cast_buf_144, sizeof(_cast_buf_144), "%d", gcd_r(48, 18));
    printf("%s\n", __ul_strcat("gcd_r(48,18)=", _cast_buf_144));
    printf("%s\n", "=== L. NUMTHEORY ===");
    static char _cast_buf_145[64];
    snprintf(_cast_buf_145, sizeof(_cast_buf_145), "%d", sum_of_digits(12345));
    printf("%s\n", __ul_strcat("sum_digits(12345)=", _cast_buf_145));
    static char _cast_buf_146[64];
    snprintf(_cast_buf_146, sizeof(_cast_buf_146), "%d", sum_of_digits(0));
    printf("%s\n", __ul_strcat("sum_digits(0)=", _cast_buf_146));
    static char _cast_buf_147[64];
    snprintf(_cast_buf_147, sizeof(_cast_buf_147), "%d", digit_count(0));
    printf("%s\n", __ul_strcat("digit_count(0)=", _cast_buf_147));
    static char _cast_buf_148[64];
    snprintf(_cast_buf_148, sizeof(_cast_buf_148), "%d", digit_count(100));
    printf("%s\n", __ul_strcat("digit_count(100)=", _cast_buf_148));
    static char _cast_buf_149[64];
    snprintf(_cast_buf_149, sizeof(_cast_buf_149), "%d", digit_count(9999));
    printf("%s\n", __ul_strcat("digit_count(9999)=", _cast_buf_149));
    static char _cast_buf_150[64];
    snprintf(_cast_buf_150, sizeof(_cast_buf_150), "%d", reverse_num(12345));
    printf("%s\n", __ul_strcat("reverse_num(12345)=", _cast_buf_150));
    static char _cast_buf_151[64];
    snprintf(_cast_buf_151, sizeof(_cast_buf_151), "%d", reverse_num((-123)));
    printf("%s\n", __ul_strcat("reverse_num(-123)=", _cast_buf_151));
    static char _cast_buf_152[64];
    snprintf(_cast_buf_152, sizeof(_cast_buf_152), "%d", reverse_num(100));
    printf("%s\n", __ul_strcat("reverse_num(100)=", _cast_buf_152));
    static char _cast_buf_153[64];
    snprintf(_cast_buf_153, sizeof(_cast_buf_153), "%d", is_perfect(6));
    printf("%s\n", __ul_strcat("is_perfect(6)=", _cast_buf_153));
    static char _cast_buf_154[64];
    snprintf(_cast_buf_154, sizeof(_cast_buf_154), "%d", is_perfect(28));
    printf("%s\n", __ul_strcat("is_perfect(28)=", _cast_buf_154));
    static char _cast_buf_155[64];
    snprintf(_cast_buf_155, sizeof(_cast_buf_155), "%d", is_perfect(12));
    printf("%s\n", __ul_strcat("is_perfect(12)=", _cast_buf_155));
    static char _cast_buf_156[64];
    snprintf(_cast_buf_156, sizeof(_cast_buf_156), "%d", is_armstrong(153));
    printf("%s\n", __ul_strcat("is_armstrong(153)=", _cast_buf_156));
    static char _cast_buf_157[64];
    snprintf(_cast_buf_157, sizeof(_cast_buf_157), "%d", is_armstrong(370));
    printf("%s\n", __ul_strcat("is_armstrong(370)=", _cast_buf_157));
    static char _cast_buf_158[64];
    snprintf(_cast_buf_158, sizeof(_cast_buf_158), "%d", is_armstrong(100));
    printf("%s\n", __ul_strcat("is_armstrong(100)=", _cast_buf_158));
    printf("%s\n", "=== M. STATS ===");
    int data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    data[0] = 4;
    data[1] = 8;
    data[2] = 15;
    data[3] = 16;
    data[4] = 23;
    data[5] = 42;
    data[6] = 7;
    data[7] = 3;
    data[8] = 11;
    data[9] = 19;
    static char _cast_buf_159[64];
    snprintf(_cast_buf_159, sizeof(_cast_buf_159), "%d", arr_sum(data, 10));
    printf("%s\n", __ul_strcat("sum=", _cast_buf_159));
    static char _cast_buf_160[64];
    snprintf(_cast_buf_160, sizeof(_cast_buf_160), "%d", arr_mean_x10(data, 10));
    printf("%s\n", __ul_strcat("mean_x10=", _cast_buf_160));
    static char _cast_buf_161[64];
    snprintf(_cast_buf_161, sizeof(_cast_buf_161), "%d", find_min(data, 10));
    static char _cast_buf_162[64];
    snprintf(_cast_buf_162, sizeof(_cast_buf_162), "%d", find_max(data, 10));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("min=", _cast_buf_161), " max="), _cast_buf_162));
    static char _cast_buf_163[64];
    snprintf(_cast_buf_163, sizeof(_cast_buf_163), "%d", arr_range(data, 10));
    printf("%s\n", __ul_strcat("range=", _cast_buf_163));
    static char _cast_buf_164[64];
    snprintf(_cast_buf_164, sizeof(_cast_buf_164), "%d", arr_variance_approx(data, 10));
    printf("%s\n", __ul_strcat("var_approx=", _cast_buf_164));
    printf("%s\n", "=== N. CAESAR ===");
    int buf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int clen = 0;
    int csum0 = 0;
    int csum1 = 0;
    int csum2 = 0;
    clen = load_str("Hello World", buf, 64);
    csum0 = ia_sum(buf, clen);
    static char _cast_buf_165[64];
    snprintf(_cast_buf_165, sizeof(_cast_buf_165), "%d", clen);
    printf("%s\n", __ul_strcat("hello_world_len=", _cast_buf_165));
    static char _cast_buf_166[64];
    snprintf(_cast_buf_166, sizeof(_cast_buf_166), "%d", csum0);
    printf("%s\n", __ul_strcat("hello_world_sum=", _cast_buf_166));
    ia_caesar_encode(buf, clen, 13);
    csum1 = ia_sum(buf, clen);
    static char _cast_buf_167[64];
    snprintf(_cast_buf_167, sizeof(_cast_buf_167), "%d", csum1);
    printf("%s\n", __ul_strcat("rot13_sum=", _cast_buf_167));
    static char _cast_buf_168[64];
    snprintf(_cast_buf_168, sizeof(_cast_buf_168), "%d", (csum1 != csum0));
    printf("%s\n", __ul_strcat("sum_changed=", _cast_buf_168));
    ia_caesar_decode(buf, clen, 13);
    csum2 = ia_sum(buf, clen);
    static char _cast_buf_169[64];
    snprintf(_cast_buf_169, sizeof(_cast_buf_169), "%d", csum2);
    printf("%s\n", __ul_strcat("decoded_sum=", _cast_buf_169));
    static char _cast_buf_170[64];
    snprintf(_cast_buf_170, sizeof(_cast_buf_170), "%d", (csum2 == csum0));
    printf("%s\n", __ul_strcat("roundtrip_ok=", _cast_buf_170));
    clen = load_str("The Quick Brown Fox", buf, 64);
    csum0 = ia_sum(buf, clen);
    static char _cast_buf_171[64];
    snprintf(_cast_buf_171, sizeof(_cast_buf_171), "%d", clen);
    printf("%s\n", __ul_strcat("tqbf_len=", _cast_buf_171));
    static char _cast_buf_172[64];
    snprintf(_cast_buf_172, sizeof(_cast_buf_172), "%d", csum0);
    printf("%s\n", __ul_strcat("tqbf_sum=", _cast_buf_172));
    ia_caesar_encode(buf, clen, 3);
    csum1 = ia_sum(buf, clen);
    static char _cast_buf_173[64];
    snprintf(_cast_buf_173, sizeof(_cast_buf_173), "%d", csum1);
    printf("%s\n", __ul_strcat("rot3_sum=", _cast_buf_173));
    static char _cast_buf_174[64];
    snprintf(_cast_buf_174, sizeof(_cast_buf_174), "%d", (csum1 != csum0));
    printf("%s\n", __ul_strcat("sum_changed=", _cast_buf_174));
    ia_caesar_decode(buf, clen, 3);
    csum2 = ia_sum(buf, clen);
    static char _cast_buf_175[64];
    snprintf(_cast_buf_175, sizeof(_cast_buf_175), "%d", csum2);
    printf("%s\n", __ul_strcat("decoded_sum=", _cast_buf_175));
    static char _cast_buf_176[64];
    snprintf(_cast_buf_176, sizeof(_cast_buf_176), "%d", (csum2 == csum0));
    printf("%s\n", __ul_strcat("roundtrip_ok=", _cast_buf_176));
    printf("%s\n", "=== O. REVERSE+PALINDROME ===");
    int rarr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int rlen = 0;
    int rsum0 = 0;
    int rsum1 = 0;
    rlen = load_str("racecar", rarr, 16);
    static char _cast_buf_177[64];
    snprintf(_cast_buf_177, sizeof(_cast_buf_177), "%d", ia_is_palindrome(rarr, rlen));
    printf("%s\n", __ul_strcat("palindrome(racecar)=", _cast_buf_177));
    rsum0 = ia_sum(rarr, rlen);
    ia_reverse(rarr, rlen);
    rsum1 = ia_sum(rarr, rlen);
    static char _cast_buf_178[64];
    snprintf(_cast_buf_178, sizeof(_cast_buf_178), "%d", (rsum0 == rsum1));
    printf("%s\n", __ul_strcat("racecar_sum_stable=", _cast_buf_178));
    static char _cast_buf_179[64];
    snprintf(_cast_buf_179, sizeof(_cast_buf_179), "%d", ia_is_palindrome(rarr, rlen));
    printf("%s\n", __ul_strcat("palindrome_after_rev=", _cast_buf_179));
    rlen = load_str("hello", rarr, 16);
    static char _cast_buf_180[64];
    snprintf(_cast_buf_180, sizeof(_cast_buf_180), "%d", ia_is_palindrome(rarr, rlen));
    printf("%s\n", __ul_strcat("palindrome(hello)=", _cast_buf_180));
    rsum0 = ia_sum(rarr, rlen);
    ia_reverse(rarr, rlen);
    rsum1 = ia_sum(rarr, rlen);
    static char _cast_buf_181[64];
    snprintf(_cast_buf_181, sizeof(_cast_buf_181), "%d", (rsum0 == rsum1));
    printf("%s\n", __ul_strcat("hello_sum_stable=", _cast_buf_181));
    static char _cast_buf_182[64];
    snprintf(_cast_buf_182, sizeof(_cast_buf_182), "%d", rarr[0]);
    printf("%s\n", __ul_strcat("hello_rev_first=", _cast_buf_182));
    static char _cast_buf_183[64];
    snprintf(_cast_buf_183, sizeof(_cast_buf_183), "%d", rarr[(rlen - 1)]);
    printf("%s\n", __ul_strcat("hello_rev_last=", _cast_buf_183));
    printf("%s\n", "=== P. RLE ===");
    int src[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int counts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int chars_out[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int len = 0;
    int np = 0;
    len = load_str("aaabbbccddddee", src, 64);
    np = rle_encode(src, len, counts, chars_out, 32);
    static char _cast_buf_184[64];
    snprintf(_cast_buf_184, sizeof(_cast_buf_184), "%d", np);
    printf("%s\n", __ul_strcat("rle1_pairs=", _cast_buf_184));
    static char _cast_buf_185[64];
    snprintf(_cast_buf_185, sizeof(_cast_buf_185), "%d", counts[0]);
    static char _cast_buf_186[64];
    snprintf(_cast_buf_186, sizeof(_cast_buf_186), "%d", chars_out[0]);
    static char _cast_buf_187[64];
    snprintf(_cast_buf_187, sizeof(_cast_buf_187), "%d", counts[1]);
    static char _cast_buf_188[64];
    snprintf(_cast_buf_188, sizeof(_cast_buf_188), "%d", chars_out[1]);
    static char _cast_buf_189[64];
    snprintf(_cast_buf_189, sizeof(_cast_buf_189), "%d", counts[2]);
    static char _cast_buf_190[64];
    snprintf(_cast_buf_190, sizeof(_cast_buf_190), "%d", chars_out[2]);
    static char _cast_buf_191[64];
    snprintf(_cast_buf_191, sizeof(_cast_buf_191), "%d", counts[3]);
    static char _cast_buf_192[64];
    snprintf(_cast_buf_192, sizeof(_cast_buf_192), "%d", chars_out[3]);
    static char _cast_buf_193[64];
    snprintf(_cast_buf_193, sizeof(_cast_buf_193), "%d", counts[4]);
    static char _cast_buf_194[64];
    snprintf(_cast_buf_194, sizeof(_cast_buf_194), "%d", chars_out[4]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("rle1: ", _cast_buf_185), " "), _cast_buf_186), " "), _cast_buf_187), " "), _cast_buf_188), " "), _cast_buf_189), " "), _cast_buf_190), " "), _cast_buf_191), " "), _cast_buf_192), " "), _cast_buf_193), " "), _cast_buf_194));
    len = load_str("abcde", src, 64);
    np = rle_encode(src, len, counts, chars_out, 32);
    static char _cast_buf_195[64];
    snprintf(_cast_buf_195, sizeof(_cast_buf_195), "%d", np);
    printf("%s\n", __ul_strcat("rle2_pairs=", _cast_buf_195));
    static char _cast_buf_196[64];
    snprintf(_cast_buf_196, sizeof(_cast_buf_196), "%d", counts[0]);
    static char _cast_buf_197[64];
    snprintf(_cast_buf_197, sizeof(_cast_buf_197), "%d", chars_out[0]);
    static char _cast_buf_198[64];
    snprintf(_cast_buf_198, sizeof(_cast_buf_198), "%d", counts[1]);
    static char _cast_buf_199[64];
    snprintf(_cast_buf_199, sizeof(_cast_buf_199), "%d", chars_out[1]);
    static char _cast_buf_200[64];
    snprintf(_cast_buf_200, sizeof(_cast_buf_200), "%d", counts[2]);
    static char _cast_buf_201[64];
    snprintf(_cast_buf_201, sizeof(_cast_buf_201), "%d", chars_out[2]);
    static char _cast_buf_202[64];
    snprintf(_cast_buf_202, sizeof(_cast_buf_202), "%d", counts[3]);
    static char _cast_buf_203[64];
    snprintf(_cast_buf_203, sizeof(_cast_buf_203), "%d", chars_out[3]);
    static char _cast_buf_204[64];
    snprintf(_cast_buf_204, sizeof(_cast_buf_204), "%d", counts[4]);
    static char _cast_buf_205[64];
    snprintf(_cast_buf_205, sizeof(_cast_buf_205), "%d", chars_out[4]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("rle2: ", _cast_buf_196), " "), _cast_buf_197), " "), _cast_buf_198), " "), _cast_buf_199), " "), _cast_buf_200), " "), _cast_buf_201), " "), _cast_buf_202), " "), _cast_buf_203), " "), _cast_buf_204), " "), _cast_buf_205));
    printf("%s\n", "=== DONE ===");
    return 0;
}


