// Stdlib/map_impl.c — Hash map FFI for UniLogic
// FNV-1a hash, open addressing with linear probing, auto-resize at 0.75 load.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAP_INIT_CAP   16
#define MAP_LOAD_MAX   75  // percent

typedef struct {
    char** keys;
    char** values;
    int*   occupied;  // 0=empty, 1=occupied, 2=tombstone
    int    capacity;
    int    count;
} ULHashMap;

// FNV-1a hash
static unsigned int fnv1a(const char* s) {
    unsigned int h = 2166136261u;
    for (; *s; s++) {
        h ^= (unsigned char)*s;
        h *= 16777619u;
    }
    return h;
}

static int find_slot(ULHashMap* m, const char* key) {
    unsigned int h = fnv1a(key) % (unsigned int)m->capacity;
    int first_tombstone = -1;
    for (int i = 0; i < m->capacity; i++) {
        int idx = (h + i) % m->capacity;
        if (m->occupied[idx] == 0) {
            return first_tombstone >= 0 ? first_tombstone : idx;
        }
        if (m->occupied[idx] == 2) {
            if (first_tombstone < 0) first_tombstone = idx;
            continue;
        }
        if (strcmp(m->keys[idx], key) == 0) return idx;
    }
    return first_tombstone >= 0 ? first_tombstone : -1;
}

static void resize(ULHashMap* m) {
    int old_cap = m->capacity;
    char** old_keys = m->keys;
    char** old_vals = m->values;
    int* old_occ = m->occupied;

    m->capacity *= 2;
    m->keys     = (char**)calloc(m->capacity, sizeof(char*));
    m->values   = (char**)calloc(m->capacity, sizeof(char*));
    m->occupied = (int*)calloc(m->capacity, sizeof(int));
    m->count    = 0;

    for (int i = 0; i < old_cap; i++) {
        if (old_occ[i] == 1) {
            int slot = find_slot(m, old_keys[i]);
            m->keys[slot]     = old_keys[i];
            m->values[slot]   = old_vals[i];
            m->occupied[slot] = 1;
            m->count++;
        }
    }
    free(old_keys);
    free(old_vals);
    free(old_occ);
}

// ── Public API ──────────────────────────────────────────────────────────

void* map_new(void) {
    ULHashMap* m = (ULHashMap*)malloc(sizeof(ULHashMap));
    m->capacity = MAP_INIT_CAP;
    m->count    = 0;
    m->keys     = (char**)calloc(MAP_INIT_CAP, sizeof(char*));
    m->values   = (char**)calloc(MAP_INIT_CAP, sizeof(char*));
    m->occupied = (int*)calloc(MAP_INIT_CAP, sizeof(int));
    return m;
}

void map_set(void* handle, const char* key, const char* val) {
    ULHashMap* m = (ULHashMap*)handle;
    if (m->count * 100 / m->capacity >= MAP_LOAD_MAX) resize(m);

    int slot = find_slot(m, key);
    if (slot < 0) return;  // should never happen after resize

    if (m->occupied[slot] == 1) {
        // Update existing
        free(m->values[slot]);
        m->values[slot] = strdup(val);
    } else {
        // Insert new
        m->keys[slot]     = strdup(key);
        m->values[slot]   = strdup(val);
        m->occupied[slot] = 1;
        m->count++;
    }
}

char* map_get(void* handle, const char* key) {
    ULHashMap* m = (ULHashMap*)handle;
    int slot = find_slot(m, key);
    if (slot >= 0 && m->occupied[slot] == 1)
        return m->values[slot];
    return "";
}

int map_has(void* handle, const char* key) {
    ULHashMap* m = (ULHashMap*)handle;
    int slot = find_slot(m, key);
    return (slot >= 0 && m->occupied[slot] == 1) ? 1 : 0;
}

void map_remove(void* handle, const char* key) {
    ULHashMap* m = (ULHashMap*)handle;
    int slot = find_slot(m, key);
    if (slot >= 0 && m->occupied[slot] == 1) {
        free(m->keys[slot]);
        free(m->values[slot]);
        m->keys[slot]     = NULL;
        m->values[slot]   = NULL;
        m->occupied[slot] = 2;  // tombstone
        m->count--;
    }
}

int map_len(void* handle) {
    ULHashMap* m = (ULHashMap*)handle;
    return m->count;
}

void map_free(void* handle) {
    ULHashMap* m = (ULHashMap*)handle;
    if (!m) return;
    for (int i = 0; i < m->capacity; i++) {
        if (m->occupied[i] == 1) {
            free(m->keys[i]);
            free(m->values[i]);
        }
    }
    free(m->keys);
    free(m->values);
    free(m->occupied);
    free(m);
}
