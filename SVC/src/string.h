#ifndef _STRING_H_
#define _STRING_H_

#include "utils.h"

char* strncpy(char* dest, const char* source, int n);
int strlen(const char* str);
int strcmp(const char* s1, const char* s2);

uint32_t is_numeric_char(char c);
uint32_t atoi(char* str);
uint32_t substring_toi(char* s, int32_t n);

uint32_t is_numeric_char(char c);
uint32_t atoi(char* str);
uint32_t substring_toi(char* s, int32_t n);

#endif
