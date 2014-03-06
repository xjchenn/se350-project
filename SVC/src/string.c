#include "string.h"

char* strncpy(char* dest, const char* source, int n) {
    char* ret = dest;
    unsigned int i;
    
    for(i = 0; i < n; i++) {
        if((*dest++ = *source++) == '\0') {
            while(++i < n) {
                *dest++ = '\0';
            }
            
            return ret;
        }
    }
    
    return ret;
}

int strlen(const char *str)
{
    const char *s;
    int i;
    for (s = str; *s; ++s);
    i = (int)(s - str);
    return i;
}
