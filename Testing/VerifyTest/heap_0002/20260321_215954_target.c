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

typedef struct node_t node_t;
typedef struct node_t {
    uint32_t hole;
    uint32_t size_c;
    node_t* next;
    node_t* prev;
} node_t;

typedef struct {
    node_t* head;
} bin_t;

void add_node(bin_t* bin, node_t* node);
node_t* get_best_fit(bin_t* bin, uint64_t size_c);
node_t* get_last_node(bin_t* bin);
int main(void);

void add_node(bin_t* bin, node_t* node)
{
    node->next = NULL;
    node->prev = NULL;
    if ((bin->head == NULL)) {
        bin->head = node;
        return;
    }
    node_t* current = bin->head;
    node_t* previous = NULL;
    while (((current != NULL) && (current->size_c <= node->size_c))) {
        previous = current;
        current = current->next;
    }
    if ((current == NULL)) {
        previous->next = node;
        node->prev = previous;
    } else {
        if ((previous != NULL)) {
            node->next = current;
            previous->next = node;
            node->prev = previous;
            current->prev = node;
        } else {
            node->next = bin->head;
            bin->head.prev = node;
            bin->head = node;
        }
    }
}

node_t* get_best_fit(bin_t* bin, uint64_t size_c)
{
    if ((bin->head == NULL)) {
        return NULL;
    }
    node_t* temp = bin->head;
    while ((temp != NULL)) {
        if ((temp->size_c >= size_c)) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

node_t* get_last_node(bin_t* bin)
{
    node_t* temp = bin->head;
    while ((temp->next != NULL)) {
        temp = temp->next;
    }
    return temp;
}

int main(void)
{
    node_t n1 = {0};
    node_t n2 = {0};
    node_t n3 = {0};
    node_t n4 = {0};
    bin_t bin = {0};
    bin.head = NULL;
    n1.hole = 0;
    n1.size_c = 32;
    n1.next = NULL;
    n1.prev = NULL;
    n2.hole = 0;
    n2.size_c = 8;
    n2.next = NULL;
    n2.prev = NULL;
    n3.hole = 0;
    n3.size_c = 128;
    n3.next = NULL;
    n3.prev = NULL;
    n4.hole = 0;
    n4.size_c = 64;
    n4.next = NULL;
    n4.prev = NULL;
    add_node((&bin), (&n1));
    add_node((&bin), (&n2));
    add_node((&bin), (&n3));
    add_node((&bin), (&n4));
    printf("%s\n", "Sorted list (ascending size):");
    node_t* cur = bin.head;
    while ((cur != NULL)) {
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%u", cur->size_c);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%u", cur->hole);
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("  size=", _cast_buf_0), " hole="), _cast_buf_1));
        cur = cur->next;
    }
    node_t* fit = {0};
    fit = get_best_fit((&bin), 10);
    __auto_type _tern_1 = 0;
    if (fit) {
        _tern_1 = fit->size_c;
    }
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", _tern_1);
    printf("%s\n", __ul_strcat("best_fit(10): size=", _cast_buf_2));
    fit = get_best_fit((&bin), 64);
    __auto_type _tern_2 = 0;
    if (fit) {
        _tern_2 = fit->size_c;
    }
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", _tern_2);
    printf("%s\n", __ul_strcat("best_fit(64): size=", _cast_buf_3));
    fit = get_best_fit((&bin), 200);
    __auto_type _tern_3 = "NULL";
    if (fit) {
        _tern_3 = "found";
    }
    printf("%s\n", __ul_strcat("best_fit(200): ", _tern_3));
    node_t* last = get_last_node((&bin));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%u", last->size_c);
    printf("%s\n", __ul_strcat("last_node: size=", _cast_buf_4));
    return 0;
}


