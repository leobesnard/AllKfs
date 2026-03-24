/*
 * =============================================================================
 *                              KFS2 BONUS - STRING UTILITIES
 * =============================================================================
 * Kernel string manipulation functions
 * Provides basic string operations for kernel use
 * =============================================================================
 */

/* =============================================================================
 *                              STRING COMPARISON
 * ============================================================================= */

/*
 * k_strcmp - Compare two null-terminated strings
 * @s1: First string
 * @s2: Second string
 *
 * Returns:
 *   0 if strings are equal
 *  <0 if s1 is lexicographically less than s2
 *  >0 if s1 is lexicographically greater than s2
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
 *                              STRING LENGTH
 * ============================================================================= */

/*
 * k_strlen - Calculate the length of a null-terminated string
 * @s: String to measure
 *
 * Returns: Number of characters before the null terminator
 */
int k_strlen(const char *s)
{
    const char *ptr = s;

    while (*ptr)
    {
        ptr++;
    }

    return ptr - s;
}

/* =============================================================================
 *                              STRING COPY
 * ============================================================================= */

/*
 * k_strcpy - Copy a string starting from a given index
 * @source: Source string to copy from
 * @start_index: Index in source to start copying from
 * @destination: Destination buffer (must be large enough)
 *
 * Returns: Pointer to destination string
 *
 * Note: If start_index is beyond the string length,
 *       destination will be an empty string
 */
char* k_strcpy(char* source, int start_index, char *destination)
{
    int length = k_strlen(source);

    /* Handle out-of-bounds start index */
    if (start_index >= length)
    {
        if (destination)
        {
            destination[0] = '\0';
        }
        return destination;
    }

    /* Copy characters from start_index to end */
    int i = 0;
    while (source[start_index] != '\0')
    {
        destination[i++] = source[start_index++];
    }

    /* Null-terminate the destination string */
    destination[i] = '\0';

    return destination;
}
