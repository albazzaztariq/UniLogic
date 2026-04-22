#include <stdio.h>
#include <stdint.h>
#include <string.h>

int str_len(char* s);
int str_contains(char* s, char* sub);
int main(void);

int str_len(char* s)
{
    return strlen(s);
}

int str_contains(char* s, char* sub)
{
    char* found = strstr(s, sub);
    return (found != NULL);
}

int main(void)
{
    char* s = "Hello World";
    int len = str_len(s);
    printf("%d\n", len);
    int has = str_contains(s, "World");
    printf("%d\n", has);
    return 0;
}


