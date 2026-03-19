#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

void greet(char* lang);
int main(void);

void greet(char* lang)
{
    if (strcmp(lang, "en") == 0) {
        printf("%s\n", "hello");
    } else if (strcmp(lang, "es") == 0) {
        printf("%s\n", "hola");
    } else if (strcmp(lang, "fr") == 0) {
        printf("%s\n", "bonjour");
    } else if (strcmp(lang, "de") == 0) {
        printf("%s\n", "hallo");
    } else {
        printf("%s\n", "unknown");
    }
}

int main(void)
{
    greet("en");
    greet("es");
    greet("fr");
    greet("de");
    greet("jp");
    char* code = "es";
    if (strcmp(code, "en") == 0) {
        printf("%s\n", "english");
    } else if (strcmp(code, "es") == 0) {
        printf("%s\n", "spanish");
    } else {
        printf("%s\n", "other");
    }
    return 0;
}


