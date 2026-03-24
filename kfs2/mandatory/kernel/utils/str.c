/**
 * =============================================================================
 * str.c - Kernel String Utility Functions
 * =============================================================================
 * 
 * Basic string manipulation functions for the kernel environment.
 * These are standalone implementations that do not depend on any
 * external libraries.
 * 
 * =============================================================================
 */

/* =============================================================================
 * String Comparison
 * ============================================================================= */

/**
 * k_strcmp - Compare two null-terminated strings
 * 
 * Performs a lexicographic comparison of two strings, comparing
 * characters as unsigned values.
 * 
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return   0 if strings are equal
 *           negative value if s1 < s2
 *           positive value if s1 > s2
 */
int k_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if ((unsigned char)*s1 != (unsigned char)*s2)
        {
            return ((unsigned char)*s1 - (unsigned char)*s2);
        }
        s1++;
        s2++;
    }

    if ((unsigned char)*s1 == '\0' && (unsigned char)*s2 == '\0')
    {
        return (0);
    }
    else if ((unsigned char)*s1 == '\0')
    {
        return (-1);
    }
    else
    {
        return (1);
    }
}

/* =============================================================================
 * String Length
 * ============================================================================= */

/**
 * k_strlen - Calculate the length of a null-terminated string
 * 
 * Counts the number of characters in a string, not including
 * the terminating null byte.
 * 
 * @param s Pointer to the string to measure
 * @return  Number of characters in the string
 */
int k_strlen(const char *s)
{
    const char *ptr;

    ptr = s;
    while (*ptr)
    {
        ptr++;
    }
    return ptr - s;
}
