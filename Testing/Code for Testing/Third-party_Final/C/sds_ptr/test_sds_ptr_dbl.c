/*
 * SDS Pointer Pattern Test 3: Double-pointer output param
 * Extracted from antirez/sds sds.c sdssplitargs() pattern.
 * Pattern: char **out_ptr is passed in; caller allocates an array and writes
 * the pointer back through the double-pointer parameter.
 * Also tests: *out_count = n pattern (int output param).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Split a comma-separated string into tokens.
 * Writes result array into *out and count into *out_count.
 * Returns 0 on success, -1 on error.
 */
int split_csv(const char *input, char ***out, int *out_count) {
    int count = 1;
    int i;
    int len = (int)strlen(input);

    /* Count commas */
    for (i = 0; i < len; i++) {
        if (input[i] == ',') count++;
    }

    char **result = (char **)malloc(count * sizeof(char *));
    if (!result) return -1;

    int slot = 0;
    int start = 0;
    for (i = 0; i <= len; i++) {
        if (input[i] == ',' || input[i] == '\0') {
            int toklen = i - start;
            result[slot] = (char *)malloc(toklen + 1);
            if (!result[slot]) { free(result); return -1; }
            memcpy(result[slot], input + start, toklen);
            result[slot][toklen] = '\0';
            slot++;
            start = i + 1;
        }
    }

    *out = result;
    *out_count = count;
    return 0;
}

int main(void) {
    char **tokens = NULL;
    int count = 0;
    int i;

    if (split_csv("alpha,beta,gamma", &tokens, &count) == 0) {
        printf("count=%d\n", count);
        for (i = 0; i < count; i++) {
            printf("token[%d]=%s\n", i, tokens[i]);
            free(tokens[i]);
        }
        free(tokens);
    }

    if (split_csv("one", &tokens, &count) == 0) {
        printf("count=%d\n", count);
        for (i = 0; i < count; i++) {
            printf("token[%d]=%s\n", i, tokens[i]);
            free(tokens[i]);
        }
        free(tokens);
    }

    return 0;
}
