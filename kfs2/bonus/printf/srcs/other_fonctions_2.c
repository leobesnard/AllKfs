/*
 * =============================================================================
 *                              KFS2 BONUS - UTILITY FUNCTIONS
 * =============================================================================
 * String and number utility functions for printf
 * =============================================================================
 */

#include "../include/ft_printf.h"

/* =============================================================================
 *                              NUMBER LENGTH FUNCTIONS
 * ============================================================================= */

/*
 * ft_intlen - Calculate the number of digits in a signed integer
 * @nb: Integer to measure
 *
 * Returns: Number of characters needed to represent the integer
 *          (including minus sign for negative numbers)
 */
int ft_intlen(int nb)
{
    int             digit_count;
    int             is_negative;
    unsigned int    number;

    if (!nb)
    {
        return (1);
    }

    digit_count = 0;

    if (nb < 0)
    {
        number = -nb;
        is_negative = 1;
    }
    else
    {
        number = nb;
        is_negative = 0;
    }

    while (number)
    {
        number = number / 10;
        digit_count++;
    }

    return (digit_count + is_negative);
}

/* =============================================================================
 *                              STRING FUNCTIONS
 * ============================================================================= */

/*
 * ft_strlen - Calculate the length of a string
 * @s: Null-terminated string
 *
 * Returns: Number of characters before the null terminator
 */
size_t ft_strlen(const char *s)
{
    size_t length;

    length = 0;
    while (s[length])
    {
        length++;
    }

    return (length);
}

/*
 * ft_strchr - Locate character in string
 * @s: String to search
 * @c: Character to find
 *
 * Returns: Pointer to first occurrence of c, or NULL if not found
 */
char *ft_strchr(const char *s, int c)
{
    while (*s)
    {
        if (*s == (char)c)
        {
            return ((char *)s);
        }
        s++;
    }

    /* Check for null terminator match */
    if (!*s && !c)
    {
        return ((char *)s);
    }

    return (NULL);
}
