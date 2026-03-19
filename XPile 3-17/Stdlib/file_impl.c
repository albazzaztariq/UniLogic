// Stdlib/file_impl.c — File I/O FFI for UniLogic
// No external dependencies — uses C stdio.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* file_read(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return strdup("");
    // Read in chunks since text mode transforms mean ftell may not match actual bytes
    int cap = 4096, total = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) { fclose(f); return strdup(""); }
    int n;
    while ((n = (int)fread(buf + total, 1, cap - total - 1, f)) > 0) {
        total += n;
        if (total >= cap - 1) { cap *= 2; buf = (char*)realloc(buf, cap); }
    }
    buf[total] = '\0';
    fclose(f);
    return buf;
}

int file_write(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;
    fputs(content, f);
    fclose(f);
    return 1;
}

int file_append(const char* path, const char* content) {
    FILE* f = fopen(path, "a");
    if (!f) return 0;
    fputs(content, f);
    fclose(f);
    return 1;
}

int file_exists(const char* path) {
    FILE* f = fopen(path, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

int file_delete(const char* path) {
    return remove(path) == 0 ? 1 : 0;
}

// Returns all lines as a single string with \n separators (C doesn't have arrays natively)
// In UL, this is parsed by the array runtime
char* file_lines(const char* path) {
    return file_read(path);  // same as file_read — line splitting done at UL level
}
