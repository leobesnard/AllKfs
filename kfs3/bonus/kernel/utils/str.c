#include "str.h"

int kstrcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        if ((uint8_t)*s1 != (uint8_t)*s2)
            return ((uint8_t)*s1 - (uint8_t)*s2);
        s1++;
        s2++;
    }
    
    if (*s1 == '\0' && *s2 == '\0')
        return 0;
    else if (*s1 == '\0')
        return -1;
    else
        return 1;
}

int kstrlen(const char *s)
{
    const char *start = s;
    while (*s)
        s++;
    return s - start;
}

char* kstrcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++))
        ;
    return dest;
}

char* kstrncpy(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

/* Convert string to integer (decimal) */
uint32_t katoi(const char *str)
{
    uint32_t result = 0;
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result;
}

/* Convert string to integer (hexadecimal) */
uint32_t katoi_hex(const char *str)
{
    uint32_t result = 0;
    
    /* Skip "0x" prefix if present */
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    
    while (*str) {
        char c = *str;
        if (c >= '0' && c <= '9') {
            result = result * 16 + (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            result = result * 16 + (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            result = result * 16 + (c - 'A' + 10);
        } else {
            break;
        }
        str++;
    }
    
    return result;
}
