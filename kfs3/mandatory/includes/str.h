#ifndef STR_H
#define STR_H

#include "types.h"

int kstrcmp(const char *s1, const char *s2);
int kstrlen(const char *s);
char* kstrcpy(char *dest, const char *src);
char* kstrncpy(char *dest, const char *src, size_t n);

#endif
