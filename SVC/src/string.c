#include "string.h"

char* strncpy(char* dest, const char* source, int n) {
    char* ret = dest;
    unsigned int i;

    for (i = 0; i < n; i++) {
        if ((*dest++ = *source++) == '\0') {
            while (++i < n) {
                *dest++ = '\0';
            }

            return ret;
        }
    }

    return ret;
}
int strlen(const char* str) {
    const char* s;
    int i;
    for (s = str; *s; ++s);
    i = (s - str);
    return i;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 == *s2++) {
        if (*s1++ == 0) {
            return (0);
        }
    }
    return (*(const unsigned char*)s1 - * (const unsigned char*)(s2 - 1));
}
