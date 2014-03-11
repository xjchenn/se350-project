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
// returns the index that the strings last matched or the length if they're identical
int strcmp(const char* s1, const char* s2) {
    int i = 0;
    while (*s1++ != '\0' && *s2++ != '\0') {
        if (*s1 != *s2) {
            return i;
        }
        i++;
    }
    return i;
}

