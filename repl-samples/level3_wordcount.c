/* Word Counter
   Counts word frequencies in a text string
   Concepts: arrays/structs, string manipulation, sorting */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* A struct to hold a word and its count */
typedef struct {
    char word[32];
    int count;
} WordEntry;

/* Find a word in the entries array, return index or -1 */
int find_word(WordEntry entries[], int num_entries, const char *word) {
    for (int i = 0; i < num_entries; i++) {
        if (strcmp(entries[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}

/* Simple insertion sort by word (alphabetical) */
void sort_entries(WordEntry entries[], int n) {
    for (int i = 1; i < n; i++) {
        WordEntry key = entries[i];
        int j = i - 1;
        while (j >= 0 && strcmp(entries[j].word, key.word) > 0) {
            entries[j + 1] = entries[j];
            j--;
        }
        entries[j + 1] = key;
    }
}

int main(void) {
    printf("=== Word Counter ===\n");
    printf("\n");

    char text[] = "the cat sat on the mat the cat liked the mat";
    printf("Text: %s\n", text);
    printf("\n");

    /* Convert to lowercase */
    for (int i = 0; text[i]; i++) {
        text[i] = tolower(text[i]);
    }

    /* Split on spaces and count words */
    WordEntry entries[64];
    int num_entries = 0;
    int total_words = 0;

    char *token = strtok(text, " ");
    while (token != NULL) {
        total_words++;
        int idx = find_word(entries, num_entries, token);
        if (idx >= 0) {
            entries[idx].count++;
        } else {
            strcpy(entries[num_entries].word, token);
            entries[num_entries].count = 1;
            num_entries++;
        }
        token = strtok(NULL, " ");
    }

    /* Sort alphabetically */
    sort_entries(entries, num_entries);

    /* Display results */
    printf("Word frequencies:\n");
    for (int i = 0; i < num_entries; i++) {
        char bar[32] = "";
        for (int j = 0; j < entries[i].count; j++) {
            bar[j] = '*';
        }
        bar[entries[i].count] = '\0';
        printf("  %-8s %d %s\n", entries[i].word, entries[i].count, bar);
    }

    printf("\n");
    printf("Unique words: %d\n", num_entries);
    printf("Total words:  %d\n", total_words);

    return 0;
}
