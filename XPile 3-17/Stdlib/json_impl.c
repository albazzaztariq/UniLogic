// Stdlib/json_impl.c — JSON parsing FFI for UniLogic
// Minimal recursive descent parser. No external dependencies.
// Supports: strings, integers, booleans, nested objects (1 level), arrays of strings/ints.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_MAX_KEYS   100
#define JSON_MAX_DEPTH  10
#define JSON_VAL_STRING 0
#define JSON_VAL_INT    1
#define JSON_VAL_BOOL   2

typedef struct {
    char  keys[JSON_MAX_KEYS][256];
    char  values[JSON_MAX_KEYS][1024];
    int   value_types[JSON_MAX_KEYS];
    int   int_values[JSON_MAX_KEYS];
    int   bool_values[JSON_MAX_KEYS];
    int   count;
} ULMap;

// ── Parser helpers ──────────────────────────────────────────────────────

static const char* skip_ws(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static const char* parse_string(const char* p, char* buf, int bufsz) {
    if (*p != '"') return NULL;
    p++;
    int i = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && *(p+1)) {
            p++;
            switch (*p) {
                case '"':  if (i < bufsz-1) buf[i++] = '"';  break;
                case '\\': if (i < bufsz-1) buf[i++] = '\\'; break;
                case 'n':  if (i < bufsz-1) buf[i++] = '\n'; break;
                case 't':  if (i < bufsz-1) buf[i++] = '\t'; break;
                case 'r':  if (i < bufsz-1) buf[i++] = '\r'; break;
                case '/':  if (i < bufsz-1) buf[i++] = '/';  break;
                default:   if (i < bufsz-1) buf[i++] = *p;   break;
            }
        } else {
            if (i < bufsz-1) buf[i++] = *p;
        }
        p++;
    }
    buf[i] = '\0';
    if (*p == '"') p++;
    return p;
}

static const char* parse_number(const char* p, long long* out) {
    char* end;
    *out = strtoll(p, &end, 10);
    return end;
}

// Skip a JSON value (for unsupported types like arrays/nested objects beyond depth)
static const char* skip_value(const char* p) {
    p = skip_ws(p);
    if (*p == '"') {
        p++;
        while (*p && *p != '"') { if (*p == '\\') p++; p++; }
        if (*p == '"') p++;
        return p;
    }
    if (*p == '{') {
        int depth = 1; p++;
        while (*p && depth > 0) {
            if (*p == '{') depth++;
            else if (*p == '}') depth--;
            else if (*p == '"') { p++; while (*p && *p != '"') { if (*p == '\\') p++; p++; } }
            p++;
        }
        return p;
    }
    if (*p == '[') {
        int depth = 1; p++;
        while (*p && depth > 0) {
            if (*p == '[') depth++;
            else if (*p == ']') depth--;
            else if (*p == '"') { p++; while (*p && *p != '"') { if (*p == '\\') p++; p++; } }
            p++;
        }
        return p;
    }
    // number, bool, null
    while (*p && *p != ',' && *p != '}' && *p != ']') p++;
    return p;
}

static const char* parse_object(const char* p, ULMap* map, int depth) {
    if (depth > JSON_MAX_DEPTH) return NULL;
    p = skip_ws(p);
    if (*p != '{') return NULL;
    p++;
    p = skip_ws(p);
    if (*p == '}') { p++; return p; }

    while (*p) {
        p = skip_ws(p);
        if (*p != '"') return NULL;
        if (map->count >= JSON_MAX_KEYS) return NULL;

        int idx = map->count;

        // Key
        p = parse_string(p, map->keys[idx], 256);
        if (!p) return NULL;

        p = skip_ws(p);
        if (*p != ':') return NULL;
        p++;
        p = skip_ws(p);

        // Value
        if (*p == '"') {
            map->value_types[idx] = JSON_VAL_STRING;
            p = parse_string(p, map->values[idx], 1024);
            if (!p) return NULL;
        } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
            map->value_types[idx] = JSON_VAL_INT;
            long long v;
            p = parse_number(p, &v);
            map->int_values[idx] = (int)v;
            snprintf(map->values[idx], 1024, "%lld", v);
        } else if (strncmp(p, "true", 4) == 0) {
            map->value_types[idx] = JSON_VAL_BOOL;
            map->bool_values[idx] = 1;
            strcpy(map->values[idx], "true");
            p += 4;
        } else if (strncmp(p, "false", 5) == 0) {
            map->value_types[idx] = JSON_VAL_BOOL;
            map->bool_values[idx] = 0;
            strcpy(map->values[idx], "false");
            p += 5;
        } else if (*p == 'n' && strncmp(p, "null", 4) == 0) {
            map->value_types[idx] = JSON_VAL_STRING;
            strcpy(map->values[idx], "null");
            p += 4;
        } else if (*p == '{') {
            // Nested object: store as string
            map->value_types[idx] = JSON_VAL_STRING;
            const char* start = p;
            p = skip_value(p);
            int len = (int)(p - start);
            if (len > 1023) len = 1023;
            memcpy(map->values[idx], start, len);
            map->values[idx][len] = '\0';
        } else if (*p == '[') {
            // Array: store as string
            map->value_types[idx] = JSON_VAL_STRING;
            const char* start = p;
            p = skip_value(p);
            int len = (int)(p - start);
            if (len > 1023) len = 1023;
            memcpy(map->values[idx], start, len);
            map->values[idx][len] = '\0';
        } else {
            return NULL;
        }
        map->count++;

        p = skip_ws(p);
        if (*p == ',') { p++; continue; }
        if (*p == '}') { p++; return p; }
        return NULL;
    }
    return NULL;
}

// ── Public API ──────────────────────────────────────────────────────────

void* json_parse(const char* s) {
    ULMap* map = (ULMap*)calloc(1, sizeof(ULMap));
    if (!map) return NULL;
    const char* end = parse_object(s, map, 0);
    if (!end) {
        fprintf(stderr, "json_parse: invalid JSON\n");
    }
    return map;
}

char* json_get(void* m, const char* key) {
    ULMap* map = (ULMap*)m;
    if (!map) return strdup("");
    for (int i = 0; i < map->count; i++) {
        if (strcmp(map->keys[i], key) == 0)
            return strdup(map->values[i]);
    }
    return strdup("");
}

int json_get_int(void* m, const char* key) {
    ULMap* map = (ULMap*)m;
    if (!map) return 0;
    for (int i = 0; i < map->count; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            if (map->value_types[i] == JSON_VAL_INT)
                return map->int_values[i];
            return atoi(map->values[i]);
        }
    }
    return 0;
}

int json_get_bool(void* m, const char* key) {
    ULMap* map = (ULMap*)m;
    if (!map) return 0;
    for (int i = 0; i < map->count; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            if (map->value_types[i] == JSON_VAL_BOOL)
                return map->bool_values[i];
            if (map->value_types[i] == JSON_VAL_STRING)
                return strcmp(map->values[i], "true") == 0;
            return map->int_values[i] != 0;
        }
    }
    return 0;
}

char* json_stringify(void* m) {
    ULMap* map = (ULMap*)m;
    if (!map || map->count == 0) return strdup("{}");

    int bufsz = 4;
    for (int i = 0; i < map->count; i++)
        bufsz += (int)strlen(map->keys[i]) + (int)strlen(map->values[i]) + 16;

    char* buf = (char*)malloc(bufsz);
    if (!buf) return strdup("{}");
    int pos = 0;
    buf[pos++] = '{';

    for (int i = 0; i < map->count; i++) {
        if (i > 0) { buf[pos++] = ','; buf[pos++] = ' '; }
        pos += snprintf(buf + pos, bufsz - pos, "\"%s\": ", map->keys[i]);
        if (map->value_types[i] == JSON_VAL_STRING)
            pos += snprintf(buf + pos, bufsz - pos, "\"%s\"", map->values[i]);
        else if (map->value_types[i] == JSON_VAL_INT)
            pos += snprintf(buf + pos, bufsz - pos, "%d", map->int_values[i]);
        else if (map->value_types[i] == JSON_VAL_BOOL)
            pos += snprintf(buf + pos, bufsz - pos, "%s", map->bool_values[i] ? "true" : "false");
    }
    buf[pos++] = '}';
    buf[pos] = '\0';
    return buf;
}

void json_free(void* m) {
    free(m);
}
