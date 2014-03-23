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

uint32_t is_numeric_char(char c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

uint32_t atoi(char* s) {
    uint32_t digits = 0;

    while (*(s + digits) != '\r' && 
           *(s + digits) != '\0') {
        digits++;
    }

    return substring_toi(s, digits);
}

uint32_t substring_toi(char* s, int32_t n) {
    uint32_t base  = 1;
    int32_t value  = 0;

    // loop from last char to first char multiplying the digit by powers of 10
    while (--n >= 0) {
        value += base * (s[n] - '0');
        base  *= 10;
    }

    return value;
}
