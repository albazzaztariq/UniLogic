/*
 * synth_bench.c — Comprehensive synthetic benchmark for c2ul round-trip testing.
 *
 * Design constraints (to work through c2ul -> UL -> codegen_c -> gcc):
 *   - No malloc/free (fixed-size stack/static arrays only)
 *   - No 2D arrays (flattened to 1D with index arithmetic)
 *   - No function pointers
 *   - No mutable char arrays (s[i] = x is not lvalue-safe through UL)
 *   - No strcpy / no char buf[] locals passed to stdlib functions
 *   - Char params always cast to int before passing to int-param functions
 *   - Self-contained: only #include <stdio.h>
 *
 * String-mutation functions (reverse, to_upper, caesar) are implemented
 * over int arrays exclusively — never over char[].
 *
 * Sections:
 *   A. Math utilities        (gcd, lcm, is_prime, sieve, fast_pow, int_sqrt)
 *   B. Sorting algorithms    (bubble, insertion, selection, counting, merge)
 *   C. Searching             (linear, binary, find_min, find_max)
 *   D. String processing     (read-only: count_digit/alpha/space, is_palindrome,
 *                             str_find, str_count, parse_uint)
 *   E. Struct operations     (Point, Rect, Triangle)
 *   F. Bit manipulation      (popcount, reverse_bits, next_pow2, bit_count_range)
 *   G. Stack (array-based)
 *   H. Queue (circular array)
 *   I. Linked list (array pool)
 *   J. Hash table (open addressing)
 *   K. Recursive algorithms  (fib, factorial, hanoi, bin_coeff, gcd_r)
 *   L. Number theory         (sum_digits, digit_count, reverse_num, is_perfect,
 *                             is_armstrong)
 *   M. Array statistics      (sum, mean_x10, min, max, range, variance_approx)
 *   N. Int-array caesar      (encode/decode on int arrays, no char mutation)
 *   O. Run-length encode     (int arrays for both counts and char codes)
 *   P. Merge sort            (with temp buffer, recursive)
 *   Q. Main harness
 */

#include <stdio.h>

/* =========================================================================
 * A. MATH UTILITIES
 * ========================================================================= */

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int lcm(int a, int b) {
    return (a / gcd(a, b)) * b;
}

int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    int i = 3;
    while (i * i <= n) {
        if (n % i == 0) return 0;
        i += 2;
    }
    return 1;
}

/* Sieve of Eratosthenes into primes[], return count */
int sieve_primes(int limit, int primes[], int max_out) {
    int sieve[200];
    int i, j, count;
    if (limit > 199) limit = 199;
    for (i = 0; i <= limit; i++) sieve[i] = 1;
    sieve[0] = 0; sieve[1] = 0;
    for (i = 2; i * i <= limit; i++) {
        if (sieve[i]) {
            for (j = i*i; j <= limit; j += i)
                sieve[j] = 0;
        }
    }
    count = 0;
    for (i = 2; i <= limit && count < max_out; i++) {
        if (sieve[i]) { primes[count] = i; count++; }
    }
    return count;
}

/* Integer square root (floor) */
int int_sqrt(int n) {
    if (n <= 0) return 0;
    int x = n;
    int y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

/* Fast integer power (base^exp) */
int fast_pow(int base, int exp) {
    int result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

/* =========================================================================
 * B. SORTING ALGORITHMS
 * ========================================================================= */

void bubble_sort(int arr[], int n) {
    int i, j, tmp;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j+1]) {
                tmp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = tmp;
            }
        }
    }
}

void insertion_sort(int arr[], int n) {
    int i, j, key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) { arr[j+1] = arr[j]; j--; }
        arr[j+1] = key;
    }
}

void selection_sort(int arr[], int n) {
    int i, j, min_idx, tmp;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i+1; j < n; j++)
            if (arr[j] < arr[min_idx]) min_idx = j;
        tmp = arr[min_idx]; arr[min_idx] = arr[i]; arr[i] = tmp;
    }
}

void counting_sort(int arr[], int n, int max_val) {
    int count[128];
    int i, j, idx;
    if (max_val > 127) max_val = 127;
    for (i = 0; i <= max_val; i++) count[i] = 0;
    for (i = 0; i < n; i++)
        if (arr[i] >= 0 && arr[i] <= max_val) count[arr[i]]++;
    idx = 0;
    for (i = 0; i <= max_val; i++)
        for (j = 0; j < count[i]; j++) { arr[idx] = i; idx++; }
}

/* Merge helpers */
void merge(int arr[], int lo, int mid, int hi, int tmp[]) {
    int i = lo, j = mid+1, k = lo;
    while (i <= mid && j <= hi) {
        if (arr[i] <= arr[j]) { tmp[k] = arr[i]; i++; }
        else                   { tmp[k] = arr[j]; j++; }
        k++;
    }
    while (i <= mid) { tmp[k] = arr[i]; i++; k++; }
    while (j <= hi)  { tmp[k] = arr[j]; j++; k++; }
    for (i = lo; i <= hi; i++) arr[i] = tmp[i];
}

void merge_sort_r(int arr[], int lo, int hi, int tmp[]) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_r(arr, lo, mid, tmp);
    merge_sort_r(arr, mid+1, hi, tmp);
    merge(arr, lo, mid, hi, tmp);
}

void merge_sort(int arr[], int n) {
    int tmp[128];
    if (n > 128) n = 128;
    merge_sort_r(arr, 0, n-1, tmp);
}

/* =========================================================================
 * C. SEARCHING
 * ========================================================================= */

int linear_search(int arr[], int n, int target) {
    int i;
    for (i = 0; i < n; i++) if (arr[i] == target) return i;
    return -1;
}

int binary_search(int arr[], int n, int target) {
    int lo = 0, hi = n-1, mid;
    while (lo <= hi) {
        mid = lo + (hi - lo) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) lo = mid+1;
        else hi = mid-1;
    }
    return -1;
}

int find_min(int arr[], int n) {
    int m = arr[0], i;
    for (i = 1; i < n; i++) if (arr[i] < m) m = arr[i];
    return m;
}

int find_max(int arr[], int n) {
    int m = arr[0], i;
    for (i = 1; i < n; i++) if (arr[i] > m) m = arr[i];
    return m;
}

/* =========================================================================
 * D. STRING PROCESSING (READ-ONLY — no s[i]=x, no char[] locals)
 * Strings passed as const char*, individual chars always cast to int.
 * ========================================================================= */

int my_strlen(const char *s) {
    int n = 0; while (s[n] != '\0') n++; return n;
}

int is_palindrome(const char *s, int len) {
    int lo = 0, hi = len-1;
    while (lo < hi) {
        if ((int)s[lo] != (int)s[hi]) return 0;
        lo++; hi--;
    }
    return 1;
}

int count_digits(const char *s) {
    int n = 0, i = 0;
    while (s[i] != '\0') {
        int c = (int)s[i];
        if (c >= 48 && c <= 57) n++;
        i++;
    }
    return n;
}

int count_alpha(const char *s) {
    int n = 0, i = 0;
    while (s[i] != '\0') {
        int c = (int)s[i];
        if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) n++;
        i++;
    }
    return n;
}

int count_spaces(const char *s) {
    int n = 0, i = 0;
    while (s[i] != '\0') { if ((int)s[i] == 32) n++; i++; }
    return n;
}

int count_char(const char *s, int c) {
    int n = 0, i = 0;
    while (s[i] != '\0') { if ((int)s[i] == c) n++; i++; }
    return n;
}

/* Parse unsigned int from string */
int parse_uint(const char *s) {
    int result = 0, i = 0;
    while (s[i] != '\0') {
        int c = (int)s[i];
        if (c >= 48 && c <= 57) result = result * 10 + (c - 48);
        i++;
    }
    return result;
}

/* Simple substring search: return index or -1 */
int str_find(const char *h, const char *n) {
    int hlen = my_strlen(h), nlen = my_strlen(n);
    int i, j, ok;
    for (i = 0; i <= hlen - nlen; i++) {
        ok = 1;
        for (j = 0; j < nlen; j++)
            if ((int)h[i+j] != (int)n[j]) { ok = 0; break; }
        if (ok) return i;
    }
    return -1;
}

/* Count non-overlapping occurrences of needle in haystack */
int str_count(const char *h, const char *n) {
    int hlen = my_strlen(h), nlen = my_strlen(n);
    int count = 0, i = 0, j, ok;
    if (nlen == 0) return 0;
    while (i <= hlen - nlen) {
        ok = 1;
        for (j = 0; j < nlen; j++)
            if ((int)h[i+j] != (int)n[j]) { ok = 0; break; }
        if (ok) { count++; i += nlen; }
        else i++;
    }
    return count;
}

/* =========================================================================
 * E. STRUCT OPERATIONS
 * ========================================================================= */

typedef struct { int x; int y; } Point;
typedef struct { int x; int y; int w; int h; } Rect;
typedef struct { int ax; int ay; int bx; int by; int cx; int cy; } Triangle;

int point_dist_sq(Point a, Point b) {
    int dx = a.x - b.x, dy = a.y - b.y;
    return dx*dx + dy*dy;
}

int rect_area(Rect r) { return r.w * r.h; }
int rect_perimeter(Rect r) { return 2 * (r.w + r.h); }

int rect_contains(Rect r, Point p) {
    return (p.x >= r.x && p.x <= r.x + r.w &&
            p.y >= r.y && p.y <= r.y + r.h);
}

int triangle_area2(Triangle t) {
    int cross = (t.bx - t.ax)*(t.cy - t.ay) - (t.cx - t.ax)*(t.by - t.ay);
    if (cross < 0) cross = -cross;
    return cross;
}

/* =========================================================================
 * F. BIT MANIPULATION
 * ========================================================================= */

int popcount(unsigned int x) {
    int count = 0;
    while (x != 0) { count += (int)(x & 1u); x >>= 1; }
    return count;
}

unsigned int reverse_bits(unsigned int x) {
    unsigned int result = 0;
    int i;
    for (i = 0; i < 32; i++) {
        result = (result << 1) | (x & 1u);
        x >>= 1;
    }
    return result;
}

unsigned int next_pow2(unsigned int n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4;
    n |= n >> 8; n |= n >> 16;
    return n + 1;
}

int bit_count_range(unsigned int lo, unsigned int hi) {
    int total = 0;
    unsigned int i = lo;
    while (i <= hi) { total += popcount(i); i++; }
    return total;
}

/* =========================================================================
 * G. STACK (array-based, capacity 32)
 * ========================================================================= */
#define STACK_CAP 32

typedef struct { int data[STACK_CAP]; int top; } Stack;

void stack_init(Stack *s) { s->top = -1; }
int stack_empty(Stack *s) { return s->top < 0; }
int stack_full(Stack *s) { return s->top >= STACK_CAP - 1; }
int stack_push(Stack *s, int v) {
    if (stack_full(s)) return 0;
    s->top++;
    s->data[s->top] = v;
    return 1;
}
int stack_pop(Stack *s) {
    if (stack_empty(s)) return -1;
    int v = s->data[s->top]; s->top--; return v;
}
int stack_peek(Stack *s) {
    if (stack_empty(s)) return -1;
    return s->data[s->top];
}
int stack_size(Stack *s) { return s->top + 1; }

/* =========================================================================
 * H. QUEUE (circular, capacity 16)
 * ========================================================================= */
#define QUEUE_CAP 16

typedef struct { int data[QUEUE_CAP]; int head; int tail; int count; } Queue;

void queue_init(Queue *q) { q->head = 0; q->tail = 0; q->count = 0; }
int queue_empty(Queue *q) { return q->count == 0; }
int queue_full(Queue *q) { return q->count == QUEUE_CAP; }
int queue_enqueue(Queue *q, int v) {
    if (queue_full(q)) return 0;
    q->data[q->tail] = v;
    q->tail = (q->tail + 1) % QUEUE_CAP;
    q->count++;
    return 1;
}
int queue_dequeue(Queue *q) {
    if (queue_empty(q)) return -1;
    int v = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_CAP;
    q->count--;
    return v;
}
int queue_front(Queue *q) {
    if (queue_empty(q)) return -1;
    return q->data[q->head];
}

/* =========================================================================
 * I. SINGLY LINKED LIST (static pool, capacity 64)
 * ========================================================================= */
#define LIST_CAP 64

typedef struct { int val; int next; } ListNode;
typedef struct { ListNode pool[LIST_CAP]; int free_head; int list_head; int size; } LinkedList;

void list_init(LinkedList *l) {
    int i;
    for (i = 0; i < LIST_CAP - 1; i++) l->pool[i].next = i+1;
    l->pool[LIST_CAP-1].next = -1;
    l->free_head = 0; l->list_head = -1; l->size = 0;
}

int list_alloc_node(LinkedList *l) {
    if (l->free_head == -1) return -1;
    int idx = l->free_head;
    l->free_head = l->pool[idx].next;
    return idx;
}

void list_free_node(LinkedList *l, int idx) {
    l->pool[idx].next = l->free_head;
    l->free_head = idx;
}

int list_insert_front(LinkedList *l, int val) {
    int idx = list_alloc_node(l);
    if (idx == -1) return 0;
    l->pool[idx].val = val;
    l->pool[idx].next = l->list_head;
    l->list_head = idx;
    l->size++;
    return 1;
}

int list_search(LinkedList *l, int val) {
    int cur = l->list_head;
    while (cur != -1) {
        if (l->pool[cur].val == val) return 1;
        cur = l->pool[cur].next;
    }
    return 0;
}

int list_delete(LinkedList *l, int val) {
    int cur = l->list_head, prev = -1;
    while (cur != -1) {
        if (l->pool[cur].val == val) {
            if (prev == -1) l->list_head = l->pool[cur].next;
            else l->pool[prev].next = l->pool[cur].next;
            list_free_node(l, cur);
            l->size--;
            return 1;
        }
        prev = cur; cur = l->pool[cur].next;
    }
    return 0;
}

int list_sum(LinkedList *l) {
    int sum = 0, cur = l->list_head;
    while (cur != -1) { sum += l->pool[cur].val; cur = l->pool[cur].next; }
    return sum;
}

/* =========================================================================
 * J. HASH TABLE (open addressing, size 31, int keys + values)
 * ========================================================================= */
#define HASH_SIZE 31

typedef struct { int keys[HASH_SIZE]; int vals[HASH_SIZE]; int used[HASH_SIZE]; } HashTable;

void ht_init(HashTable *h) {
    int i;
    for (i = 0; i < HASH_SIZE; i++) { h->keys[i]=0; h->vals[i]=0; h->used[i]=0; }
}

int ht_hash(int key) {
    int h = key % HASH_SIZE;
    if (h < 0) h += HASH_SIZE;
    return h;
}

int ht_set(HashTable *h, int key, int val) {
    int i, idx = ht_hash(key);
    for (i = 0; i < HASH_SIZE; i++) {
        int slot = (idx + i) % HASH_SIZE;
        if (!h->used[slot] || h->keys[slot] == key) {
            h->keys[slot]=key; h->vals[slot]=val; h->used[slot]=1; return 1;
        }
    }
    return 0;
}

int ht_get(HashTable *h, int key, int *out) {
    int i, idx = ht_hash(key);
    for (i = 0; i < HASH_SIZE; i++) {
        int slot = (idx + i) % HASH_SIZE;
        if (!h->used[slot]) return 0;
        if (h->keys[slot] == key) { *out = h->vals[slot]; return 1; }
    }
    return 0;
}

/* =========================================================================
 * K. RECURSIVE ALGORITHMS
 * ========================================================================= */

int fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n-1);
}

int hanoi_moves(int n) {
    if (n == 0) return 0;
    return 2 * hanoi_moves(n-1) + 1;
}

int bin_coeff(int n, int k) {
    if (k == 0 || k == n) return 1;
    if (k > n-k) k = n-k;
    int result = 1, i;
    for (i = 0; i < k; i++) result = result * (n-i) / (i+1);
    return result;
}

int gcd_r(int a, int b) {
    if (b == 0) return a;
    return gcd_r(b, a % b);
}

/* =========================================================================
 * L. NUMBER THEORY
 * ========================================================================= */

int sum_of_digits(int n) {
    if (n < 0) n = -n;
    int s = 0;
    if (n == 0) return 0;
    while (n > 0) { s += n % 10; n /= 10; }
    return s;
}

int digit_count(int n) {
    if (n == 0) return 1;
    if (n < 0) n = -n;
    int c = 0;
    while (n > 0) { c++; n /= 10; }
    return c;
}

int reverse_num(int n) {
    int neg = 0, r = 0;
    if (n < 0) { neg = 1; n = -n; }
    while (n > 0) { r = r * 10 + n % 10; n /= 10; }
    return neg ? -r : r;
}

int is_perfect(int n) {
    if (n < 2) return 0;
    int sum = 1, i;
    for (i = 2; i*i <= n; i++) {
        if (n % i == 0) {
            sum += i;
            if (i != n/i) sum += n/i;
        }
    }
    return sum == n;
}

int is_armstrong(int n) {
    int orig = n, dc = digit_count(n), s = 0, d;
    while (n > 0) { d = n % 10; s += fast_pow(d, dc); n /= 10; }
    return s == orig;
}

/* =========================================================================
 * M. ARRAY STATISTICS
 * ========================================================================= */

int arr_sum(int arr[], int n) {
    int s = 0, i;
    for (i = 0; i < n; i++) s += arr[i];
    return s;
}

int arr_mean_x10(int arr[], int n) {
    if (n == 0) return 0;
    return (arr_sum(arr, n) * 10) / n;
}

int arr_range(int arr[], int n) {
    return find_max(arr, n) - find_min(arr, n);
}

int arr_variance_approx(int arr[], int n) {
    if (n == 0) return 0;
    int mean = arr_sum(arr, n) / n;
    int sv = 0, i, d;
    for (i = 0; i < n; i++) { d = arr[i] - mean; sv += d*d; }
    return sv / n;
}

/* =========================================================================
 * N. INT-ARRAY CAESAR
 * Works on int arrays (each element is a char code) — no char mutation.
 * load_str: copy a string literal into an int array, return length.
 * ========================================================================= */

int load_str(const char *src, int dst[], int max) {
    int i = 0;
    while (src[i] != '\0' && i < max-1) {
        dst[i] = (int)src[i];
        i++;
    }
    dst[i] = 0;
    return i;
}

void ia_caesar_encode(int arr[], int len, int shift) {
    int i;
    shift = ((shift % 26) + 26) % 26;
    for (i = 0; i < len; i++) {
        int c = arr[i];
        if (c >= 65 && c <= 90)
            arr[i] = 65 + (c - 65 + shift) % 26;
        else if (c >= 97 && c <= 122)
            arr[i] = 97 + (c - 97 + shift) % 26;
    }
}

void ia_caesar_decode(int arr[], int len, int shift) {
    ia_caesar_encode(arr, len, 26 - shift);
}

/* Compute sum of int array (used as a checksum for verification) */
int ia_sum(int arr[], int len) {
    int s = 0, i;
    for (i = 0; i < len; i++) s += arr[i];
    return s;
}

/* =========================================================================
 * O. INT-ARRAY REVERSE + IS_PALINDROME
 * ========================================================================= */

void ia_reverse(int arr[], int len) {
    int lo = 0, hi = len-1, tmp;
    while (lo < hi) {
        tmp = arr[lo]; arr[lo] = arr[hi]; arr[hi] = tmp;
        lo++; hi--;
    }
}

int ia_is_palindrome(int arr[], int len) {
    int lo = 0, hi = len-1;
    while (lo < hi) {
        if (arr[lo] != arr[hi]) return 0;
        lo++; hi--;
    }
    return 1;
}

/* =========================================================================
 * P. RUN-LENGTH ENCODE (over int arrays)
 * ========================================================================= */

int rle_encode(int src[], int slen, int counts[], int chars[], int max_pairs) {
    int n = 0, i = 0;
    while (i < slen && n < max_pairs) {
        int cur = src[i], cnt = 1;
        while (i + cnt < slen && src[i+cnt] == cur) cnt++;
        counts[n] = cnt;
        chars[n] = cur;
        n++;
        i += cnt;
    }
    return n;
}

/* =========================================================================
 * Q. MAIN HARNESS
 * ========================================================================= */

int main(void) {
    int i;

    /* --- A: Math --- */
    printf("=== A. MATH ===\n");
    printf("gcd(48,18)=%d\n", gcd(48, 18));
    printf("gcd(100,75)=%d\n", gcd(100, 75));
    printf("lcm(4,6)=%d\n", lcm(4, 6));
    printf("lcm(12,18)=%d\n", lcm(12, 18));
    printf("is_prime(1)=%d\n", is_prime(1));
    printf("is_prime(2)=%d\n", is_prime(2));
    printf("is_prime(17)=%d\n", is_prime(17));
    printf("is_prime(100)=%d\n", is_prime(100));
    printf("is_prime(97)=%d\n", is_prime(97));
    printf("int_sqrt(0)=%d\n", int_sqrt(0));
    printf("int_sqrt(9)=%d\n", int_sqrt(9));
    printf("int_sqrt(10)=%d\n", int_sqrt(10));
    printf("int_sqrt(144)=%d\n", int_sqrt(144));
    printf("fast_pow(2,10)=%d\n", fast_pow(2, 10));
    printf("fast_pow(3,5)=%d\n", fast_pow(3, 5));
    printf("fast_pow(7,0)=%d\n", fast_pow(7, 0));

    {
        int primes[25];
        int pc = sieve_primes(100, primes, 25);
        printf("primes<=100: count=%d\n", pc);
        printf("first5: %d %d %d %d %d\n", primes[0], primes[1], primes[2], primes[3], primes[4]);
        printf("last5: %d %d %d %d %d\n", primes[pc-5], primes[pc-4], primes[pc-3], primes[pc-2], primes[pc-1]);
    }

    /* --- B: Sort --- */
    printf("=== B. SORT ===\n");
    {
        int a1[8];
        a1[0]=64; a1[1]=34; a1[2]=25; a1[3]=12; a1[4]=22; a1[5]=11; a1[6]=90; a1[7]=1;
        bubble_sort(a1, 8);
        printf("bubble: %d %d %d %d %d %d %d %d\n",
               a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6], a1[7]);

        int a2[8];
        a2[0]=5; a2[1]=3; a2[2]=8; a2[3]=1; a2[4]=9; a2[5]=2; a2[6]=7; a2[7]=4;
        insertion_sort(a2, 8);
        printf("insertion: %d %d %d %d %d %d %d %d\n",
               a2[0], a2[1], a2[2], a2[3], a2[4], a2[5], a2[6], a2[7]);

        int a3[8];
        a3[0]=10; a3[1]=7; a3[2]=3; a3[3]=9; a3[4]=1; a3[5]=5; a3[6]=6; a3[7]=2;
        selection_sort(a3, 8);
        printf("selection: %d %d %d %d %d %d %d %d\n",
               a3[0], a3[1], a3[2], a3[3], a3[4], a3[5], a3[6], a3[7]);

        int a4[10];
        a4[0]=4; a4[1]=2; a4[2]=7; a4[3]=1; a4[4]=5; a4[5]=3; a4[6]=6; a4[7]=0; a4[8]=9; a4[9]=8;
        counting_sort(a4, 10, 9);
        printf("counting: %d %d %d %d %d %d %d %d %d %d\n",
               a4[0], a4[1], a4[2], a4[3], a4[4], a4[5], a4[6], a4[7], a4[8], a4[9]);

        int a5[12];
        a5[0]=38; a5[1]=27; a5[2]=43; a5[3]=3; a5[4]=9; a5[5]=82; a5[6]=10; a5[7]=1;
        a5[8]=100; a5[9]=55; a5[10]=44; a5[11]=22;
        merge_sort(a5, 12);
        printf("merge: %d %d %d %d %d %d %d %d %d %d %d %d\n",
               a5[0], a5[1], a5[2], a5[3], a5[4], a5[5], a5[6], a5[7], a5[8], a5[9], a5[10], a5[11]);
    }

    /* --- C: Search --- */
    printf("=== C. SEARCH ===\n");
    {
        int arr[10];
        for (i = 0; i < 10; i++) arr[i] = i * 3 + 1;
        printf("linear(13)=%d\n", linear_search(arr, 10, 13));
        printf("linear(5)=%d\n", linear_search(arr, 10, 5));
        printf("binary(19)=%d\n", binary_search(arr, 10, 19));
        printf("binary(2)=%d\n", binary_search(arr, 10, 2));
        printf("min=%d max=%d\n", find_min(arr, 10), find_max(arr, 10));
    }

    /* --- D: Strings (read-only) --- */
    printf("=== D. STRINGS ===\n");
    {
        const char *str_d1 = "Hello World 123!";
        printf("len=%d\n", my_strlen(str_d1));
        printf("digits=%d\n", count_digits(str_d1));
        printf("alpha=%d\n", count_alpha(str_d1));
        printf("spaces=%d\n", count_spaces(str_d1));
        printf("count_l=%d\n", count_char(str_d1, 108));  /* 'l'=108 */

        printf("palindrome(racecar)=%d\n", is_palindrome("racecar", 7));
        printf("palindrome(hello)=%d\n",   is_palindrome("hello",   5));
        printf("palindrome(abcba)=%d\n",   is_palindrome("abcba",   5));

        printf("parse_uint(4291)=%d\n",  parse_uint("4291"));
        printf("parse_uint(0)=%d\n",     parse_uint("0"));
        printf("parse_uint(99999)=%d\n", parse_uint("99999"));

        printf("str_find(the_cat_sat,cat)=%d\n", str_find("the cat sat", "cat"));
        printf("str_find(hello,xyz)=%d\n",        str_find("hello", "xyz"));
        printf("str_find(abcabc,bc)=%d\n",         str_find("abcabc", "bc"));
        printf("str_count(banana,an)=%d\n",         str_count("banana", "an"));
        printf("str_count(aaa,aa)=%d\n",             str_count("aaa", "aa"));
        printf("str_count(abcabc,abc)=%d\n",         str_count("abcabc", "abc"));
    }

    /* --- E: Structs --- */
    printf("=== E. STRUCTS ===\n");
    {
        Point a; a.x=3; a.y=4;
        Point b; b.x=0; b.y=0;
        printf("dist_sq((3,4),(0,0))=%d\n", point_dist_sq(a, b));

        Rect r; r.x=0; r.y=0; r.w=10; r.h=5;
        printf("rect_area=%d\n", rect_area(r));
        printf("rect_perim=%d\n", rect_perimeter(r));

        Point p1; p1.x=5; p1.y=3;
        Point p2; p2.x=11; p2.y=3;
        printf("contains(5,3)=%d\n",  rect_contains(r, p1));
        printf("contains(11,3)=%d\n", rect_contains(r, p2));

        Triangle t;
        t.ax=0; t.ay=0; t.bx=4; t.by=0; t.cx=0; t.cy=3;
        printf("tri_area2=%d\n", triangle_area2(t));
    }

    /* --- F: Bits --- */
    printf("=== F. BITS ===\n");
    printf("popcount(0)=%d\n",         popcount(0));
    printf("popcount(255)=%d\n",       popcount(255));
    printf("popcount(65535)=%d\n",     popcount(65535));
    printf("popcount(0x80000001)=%d\n", popcount(0x80000001u));
    printf("next_pow2(0)=%u\n",   next_pow2(0));
    printf("next_pow2(1)=%u\n",   next_pow2(1));
    printf("next_pow2(5)=%u\n",   next_pow2(5));
    printf("next_pow2(8)=%u\n",   next_pow2(8));
    printf("next_pow2(9)=%u\n",   next_pow2(9));
    printf("bit_range(0,7)=%d\n",  bit_count_range(0, 7));
    printf("bit_range(1,4)=%d\n",  bit_count_range(1, 4));
    printf("rev_bits(1)=%u\n",        reverse_bits(1u));
    printf("rev_bits(0x80000000)=%u\n", reverse_bits(0x80000000u));

    /* --- G: Stack --- */
    printf("=== G. STACK ===\n");
    {
        Stack stk;
        stack_init(&stk);
        printf("empty=%d\n", stack_empty(&stk));
        for (i = 1; i <= 5; i++) stack_push(&stk, i*10);
        printf("size=%d\n", stack_size(&stk));
        printf("peek=%d\n", stack_peek(&stk));
        while (!stack_empty(&stk)) printf("pop=%d\n", stack_pop(&stk));
        printf("empty=%d\n", stack_empty(&stk));
    }

    /* --- H: Queue --- */
    printf("=== H. QUEUE ===\n");
    {
        Queue q;
        queue_init(&q);
        printf("empty=%d\n", queue_empty(&q));
        for (i = 0; i < 5; i++) queue_enqueue(&q, i*7+1);
        printf("count=%d\n", q.count);
        printf("front=%d\n", queue_front(&q));
        while (!queue_empty(&q)) printf("deq=%d\n", queue_dequeue(&q));
        printf("empty=%d\n", queue_empty(&q));
    }

    /* --- I: Linked List --- */
    printf("=== I. LIST ===\n");
    {
        LinkedList lst;
        list_init(&lst);
        for (i = 1; i <= 6; i++) list_insert_front(&lst, i*5);
        printf("size=%d\n", lst.size);
        printf("sum=%d\n", list_sum(&lst));
        printf("search(15)=%d\n", list_search(&lst, 15));
        printf("search(99)=%d\n", list_search(&lst, 99));
        list_delete(&lst, 15);
        printf("after_del size=%d\n", lst.size);
        printf("after_del sum=%d\n", list_sum(&lst));
    }

    /* --- J: Hash Table --- */
    printf("=== J. HASH ===\n");
    {
        HashTable ht;
        ht_init(&ht);
        for (i = 0; i < 10; i++) ht_set(&ht, i*7+3, i*i+1);
        int out;
        for (i = 0; i < 10; i++) {
            int found = ht_get(&ht, i*7+3, &out);
            printf("ht[%d]=%d found=%d\n", i*7+3, found ? out : -1, found);
        }
        printf("ht[999] found=%d\n", ht_get(&ht, 999, &out));
    }

    /* --- K: Recursion --- */
    printf("=== K. RECURSION ===\n");
    for (i = 0; i <= 10; i++) printf("fib(%d)=%d\n", i, fib(i));
    for (i = 0; i <= 8; i++) printf("fact(%d)=%d\n", i, factorial(i));
    for (i = 0; i <= 6; i++) printf("hanoi(%d)=%d\n", i, hanoi_moves(i));
    printf("C(5,2)=%d\n",  bin_coeff(5, 2));
    printf("C(10,3)=%d\n", bin_coeff(10, 3));
    printf("C(6,6)=%d\n",  bin_coeff(6, 6));
    printf("gcd_r(48,18)=%d\n", gcd_r(48, 18));

    /* --- L: Number theory --- */
    printf("=== L. NUMTHEORY ===\n");
    printf("sum_digits(12345)=%d\n", sum_of_digits(12345));
    printf("sum_digits(0)=%d\n",     sum_of_digits(0));
    printf("digit_count(0)=%d\n",    digit_count(0));
    printf("digit_count(100)=%d\n",  digit_count(100));
    printf("digit_count(9999)=%d\n", digit_count(9999));
    printf("reverse_num(12345)=%d\n",  reverse_num(12345));
    printf("reverse_num(-123)=%d\n",   reverse_num(-123));
    printf("reverse_num(100)=%d\n",    reverse_num(100));
    printf("is_perfect(6)=%d\n",   is_perfect(6));
    printf("is_perfect(28)=%d\n",  is_perfect(28));
    printf("is_perfect(12)=%d\n",  is_perfect(12));
    printf("is_armstrong(153)=%d\n", is_armstrong(153));
    printf("is_armstrong(370)=%d\n", is_armstrong(370));
    printf("is_armstrong(100)=%d\n", is_armstrong(100));

    /* --- M: Stats --- */
    printf("=== M. STATS ===\n");
    {
        int data[10];
        data[0]=4; data[1]=8; data[2]=15; data[3]=16; data[4]=23;
        data[5]=42; data[6]=7; data[7]=3; data[8]=11; data[9]=19;
        printf("sum=%d\n",        arr_sum(data, 10));
        printf("mean_x10=%d\n",   arr_mean_x10(data, 10));
        printf("min=%d max=%d\n", find_min(data, 10), find_max(data, 10));
        printf("range=%d\n",      arr_range(data, 10));
        printf("var_approx=%d\n", arr_variance_approx(data, 10));
    }

    /* --- N: Int-array Caesar --- */
    /* Verify via checksum: sum of char codes before == sum after decode. */
    printf("=== N. CAESAR ===\n");
    {
        int buf[64];
        int clen;
        int csum0, csum1, csum2;

        clen = load_str("Hello World", buf, 64);
        csum0 = ia_sum(buf, clen);
        printf("hello_world_len=%d\n", clen);
        printf("hello_world_sum=%d\n", csum0);
        ia_caesar_encode(buf, clen, 13);
        csum1 = ia_sum(buf, clen);
        printf("rot13_sum=%d\n", csum1);
        printf("sum_changed=%d\n", csum1 != csum0);
        ia_caesar_decode(buf, clen, 13);
        csum2 = ia_sum(buf, clen);
        printf("decoded_sum=%d\n", csum2);
        printf("roundtrip_ok=%d\n", csum2 == csum0);

        clen = load_str("The Quick Brown Fox", buf, 64);
        csum0 = ia_sum(buf, clen);
        printf("tqbf_len=%d\n", clen);
        printf("tqbf_sum=%d\n", csum0);
        ia_caesar_encode(buf, clen, 3);
        csum1 = ia_sum(buf, clen);
        printf("rot3_sum=%d\n", csum1);
        printf("sum_changed=%d\n", csum1 != csum0);
        ia_caesar_decode(buf, clen, 3);
        csum2 = ia_sum(buf, clen);
        printf("decoded_sum=%d\n", csum2);
        printf("roundtrip_ok=%d\n", csum2 == csum0);
    }

    /* --- O: Int-array reverse + palindrome --- */
    printf("=== O. REVERSE+PALINDROME ===\n");
    {
        int rarr[16];
        int rlen;
        int rsum0, rsum1;

        rlen = load_str("racecar", rarr, 16);
        printf("palindrome(racecar)=%d\n", ia_is_palindrome(rarr, rlen));
        rsum0 = ia_sum(rarr, rlen);
        ia_reverse(rarr, rlen);
        rsum1 = ia_sum(rarr, rlen);
        printf("racecar_sum_stable=%d\n", rsum0 == rsum1);
        printf("palindrome_after_rev=%d\n", ia_is_palindrome(rarr, rlen));

        rlen = load_str("hello", rarr, 16);
        printf("palindrome(hello)=%d\n", ia_is_palindrome(rarr, rlen));
        rsum0 = ia_sum(rarr, rlen);
        ia_reverse(rarr, rlen);
        rsum1 = ia_sum(rarr, rlen);
        printf("hello_sum_stable=%d\n", rsum0 == rsum1);
        printf("hello_rev_first=%d\n", rarr[0]);    /* 'o'=111 */
        printf("hello_rev_last=%d\n", rarr[rlen-1]); /* 'h'=104 */
    }

    /* --- P: RLE on int arrays --- */
    /* Print counts and char-codes (as integers) for deterministic output */
    printf("=== P. RLE ===\n");
    {
        int src[64], counts[32], chars_out[32];
        int len, np;

        len = load_str("aaabbbccddddee", src, 64);
        np = rle_encode(src, len, counts, chars_out, 32);
        printf("rle1_pairs=%d\n", np);
        printf("rle1: %d %d %d %d %d %d %d %d %d %d\n",
               counts[0], chars_out[0],
               counts[1], chars_out[1],
               counts[2], chars_out[2],
               counts[3], chars_out[3],
               counts[4], chars_out[4]);

        len = load_str("abcde", src, 64);
        np = rle_encode(src, len, counts, chars_out, 32);
        printf("rle2_pairs=%d\n", np);
        printf("rle2: %d %d %d %d %d %d %d %d %d %d\n",
               counts[0], chars_out[0],
               counts[1], chars_out[1],
               counts[2], chars_out[2],
               counts[3], chars_out[3],
               counts[4], chars_out[4]);
    }

    printf("=== DONE ===\n");
    return 0;
}
