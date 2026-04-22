/* test_redis_adlist.c
 * Extracted from Redis adlist.c: doubly linked list operations.
 * Simplified: no function pointers in the list struct (dup/free/match removed).
 * Tests: create, addHead, addTail, insertNode, delNode, index, rotate, join.
 */
#include <stdio.h>
#include <stdlib.h>

#define AL_START_HEAD 0
#define AL_START_TAIL 1

typedef struct listNode {
    struct listNode *prev;
    struct listNode *next;
    void *value;
} listNode;

typedef struct listIter {
    listNode *next;
    int direction;
} listIter;

typedef struct list {
    listNode *head;
    listNode *tail;
    unsigned long len;
} list;

/* ---------- core functions (adapted from adlist.c) ---------- */

list *listCreate(void) {
    list *l = (list *)malloc(sizeof(*l));
    if (!l) return NULL;
    l->head = l->tail = NULL;
    l->len = 0;
    return l;
}

void listEmpty(list *l) {
    unsigned long len;
    listNode *current, *next;
    current = l->head;
    len = l->len;
    while (len--) {
        next = current->next;
        free(current);
        current = next;
    }
    l->head = l->tail = NULL;
    l->len = 0;
}

void listRelease(list *l) {
    if (!l) return;
    listEmpty(l);
    free(l);
}

void listLinkNodeHead(list *l, listNode *node) {
    if (l->len == 0) {
        l->head = l->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = l->head;
        l->head->prev = node;
        l->head = node;
    }
    l->len++;
}

void listLinkNodeTail(list *l, listNode *node) {
    if (l->len == 0) {
        l->head = l->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = l->tail;
        node->next = NULL;
        l->tail->next = node;
        l->tail = node;
    }
    l->len++;
}

list *listAddNodeHead(list *l, void *value) {
    listNode *node = (listNode *)malloc(sizeof(*node));
    if (!node) return NULL;
    node->value = value;
    listLinkNodeHead(l, node);
    return l;
}

list *listAddNodeTail(list *l, void *value) {
    listNode *node = (listNode *)malloc(sizeof(*node));
    if (!node) return NULL;
    node->value = value;
    listLinkNodeTail(l, node);
    return l;
}

list *listInsertNode(list *l, listNode *old_node, void *value, int after) {
    listNode *node = (listNode *)malloc(sizeof(*node));
    if (!node) return NULL;
    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (l->tail == old_node) l->tail = node;
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        if (l->head == old_node) l->head = node;
    }
    if (node->prev != NULL) node->prev->next = node;
    if (node->next != NULL) node->next->prev = node;
    l->len++;
    return l;
}

void listUnlinkNode(list *l, listNode *node) {
    if (node->prev)
        node->prev->next = node->next;
    else
        l->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        l->tail = node->prev;
    node->next = NULL;
    node->prev = NULL;
    l->len--;
}

void listDelNode(list *l, listNode *node) {
    listUnlinkNode(l, node);
    free(node);
}

void listRewind(list *l, listIter *li) {
    li->next = l->head;
    li->direction = AL_START_HEAD;
}

listNode *listNext(listIter *iter) {
    listNode *current = iter->next;
    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

listNode *listIndex(list *l, long index) {
    listNode *n;
    if (index < 0) {
        index = (-index) - 1;
        n = l->tail;
        while (index-- && n) n = n->prev;
    } else {
        n = l->head;
        while (index-- && n) n = n->next;
    }
    return n;
}

void listRotateTailToHead(list *l) {
    if (l->len <= 1) return;
    listNode *tail = l->tail;
    l->tail = tail->prev;
    l->tail->next = NULL;
    l->head->prev = tail;
    tail->prev = NULL;
    tail->next = l->head;
    l->head = tail;
}

void listJoin(list *l, list *o) {
    if (o->len == 0) return;
    o->head->prev = l->tail;
    if (l->tail)
        l->tail->next = o->head;
    else
        l->head = o->head;
    l->tail = o->tail;
    l->len += o->len;
    o->head = o->tail = NULL;
    o->len = 0;
}

/* ---------- print helpers ---------- */

static void printList(list *l) {
    listIter iter;
    listNode *node;
    listRewind(l, &iter);
    printf("[");
    int first = 1;
    while ((node = listNext(&iter)) != NULL) {
        if (!first) printf(", ");
        printf("%d", (int)(long)node->value);
        first = 0;
    }
    printf("] len=%lu\n", l->len);
}

/* ---------- main ---------- */

int main(void)
{
    list *l = listCreate();

    /* Add 1,2,3 to tail */
    listAddNodeTail(l, (void*)1);
    listAddNodeTail(l, (void*)2);
    listAddNodeTail(l, (void*)3);
    printList(l);

    /* Add 0 to head */
    listAddNodeHead(l, (void*)0);
    printList(l);

    /* Insert 99 after index 1 (value=1) */
    listNode *n1 = listIndex(l, 1);
    listInsertNode(l, n1, (void*)99, 1);
    printList(l);

    /* Delete node at index 2 (value=99) */
    listNode *n2 = listIndex(l, 2);
    listDelNode(l, n2);
    printList(l);

    /* Index lookup: 0-based from head, negative from tail */
    printf("index 0: %d\n", (int)(long)listIndex(l, 0)->value);
    printf("index -1: %d\n", (int)(long)listIndex(l, -1)->value);
    printf("index 2: %d\n", (int)(long)listIndex(l, 2)->value);

    /* Rotate tail to head */
    listRotateTailToHead(l);
    printList(l);

    /* Join two lists */
    list *l2 = listCreate();
    listAddNodeTail(l2, (void*)10);
    listAddNodeTail(l2, (void*)11);
    listJoin(l, l2);
    printList(l);
    printf("l2 len after join: %lu\n", l2->len);

    listRelease(l);
    listRelease(l2);
    return 0;
}
