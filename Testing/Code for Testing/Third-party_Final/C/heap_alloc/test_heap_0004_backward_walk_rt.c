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
    int hole;
    int size_c;
    node_t* next;
    node_t* prev;
} node_t;

typedef struct {
    node_t* head;
} bin_t;

void add_node(bin_t* bin, node_t* node);
void remove_node(bin_t* bin, node_t* node);
void print_backward(bin_t* bin);
void print_forward(bin_t* bin);
int main(void);

void add_node(bin_t* bin, node_t* node)
{
    node->next = ((void)0);
    node->prev = ((void)0);
    if ((bin->head == ((void)0))) {
        bin->head = node;
        return;
    }
    node_t* current = bin->head;
    node_t* previous = ((void)0);
    while (((current != ((void)0)) && (current->size_c <= node->size_c))) {
        previous = current;
        current = current->next;
    }
    if ((current == ((void)0))) {
        previous->next = node;
        node->prev = previous;
    } else {
        if ((previous != ((void)0))) {
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

void remove_node(bin_t* bin, node_t* node)
{
    if ((bin->head == ((void)0))) {
        return;
    }
    if ((bin->head == node)) {
        bin->head = bin->head.next;
        return;
    }
    node_t* temp = bin->head.next;
    while ((temp != ((void)0))) {
        if ((temp == node)) {
            if ((temp->next == ((void)0))) {
                temp->prev.next = ((void)0);
            } else {
                temp->prev.next = temp->next;
                temp->next.prev = temp->prev;
            }
            return;
        }
        temp = temp->next;
    }
}

void print_backward(bin_t* bin)
{
    if ((bin->head == ((void)0))) {
        printf("%s\n", "(empty)");
        return;
    }
    node_t* cur = bin->head;
    while ((cur->next != ((void)0))) {
        cur = cur->next;
    }
    printf("%s\n", "backward:");
    while ((cur != ((void)0))) {
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", cur->size_c);
        printf("%s\n", __ul_strcat(" ", _cast_buf_0));
        cur = cur->prev;
    }
    printf("%s\n", "");
}

void print_forward(bin_t* bin)
{
    printf("%s\n", "forward:");
    node_t* cur = bin->head;
    while ((cur != ((void)0))) {
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", cur->size_c);
        printf("%s\n", __ul_strcat(" ", _cast_buf_1));
        cur = cur->next;
    }
    printf("%s\n", "");
}

int main(void)
{
    node_t n1 = {0};
    node_t n2 = {0};
    node_t n3 = {0};
    node_t n4 = {0};
    node_t n5 = {0};
    bin_t bin = {0};
    bin.head = ((void)0);
    n1.size_c = 16;
    n1.hole = 1;
    n2.size_c = 32;
    n2.hole = 1;
    n3.size_c = 64;
    n3.hole = 1;
    n4.size_c = 128;
    n4.hole = 1;
    n5.size_c = 256;
    n5.hole = 1;
    add_node((&bin), (&n3));
    add_node((&bin), (&n1));
    add_node((&bin), (&n5));
    add_node((&bin), (&n2));
    add_node((&bin), (&n4));
    printf("%s\n", "Initial state:");
    print_forward((&bin));
    print_backward((&bin));
    printf("%s\n", "\nAfter remove head (size=16):");
    remove_node((&bin), (&n1));
    print_forward((&bin));
    print_backward((&bin));
    printf("%s\n", "\nAfter remove middle (size=64):");
    remove_node((&bin), (&n3));
    print_forward((&bin));
    print_backward((&bin));
    printf("%s\n", "\nAfter remove tail (size=256):");
    remove_node((&bin), (&n5));
    print_forward((&bin));
    print_backward((&bin));
    printf("%s\n", "\nPrev-pointer chain verification:");
    node_t* cur = bin.head;
    node_t* prev_saved = ((void)0);
    while ((cur != ((void)0))) {
        __auto_type _tern_1 = "NULL";
        if (cur->prev) {
            _tern_1 = "ok";
        }
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", cur->size_c);
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("  node size=", _cast_buf_2), " prev_size="), _tern_1));
        prev_saved = cur;
        cur = cur->next;
    }
    return 0;
}


