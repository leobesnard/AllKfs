/* ************************************************************************** */
/*                                                                            */
/*   other_fonctions_2.c - String and length utility functions                */
/*                                                                            */
/*   Implements string length, integer length, and string search functions    */
/*   used by the printf implementation.                                       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_printf.h"

/* ==========================================================================
   Integer Digit Count
   ========================================================================== */

int
ft_intlen(int nb)
{
    int             count;
    int             is_negative;
    unsigned int    number;

    /* Zero has length 1 */
    if (!nb)
    {
        return (1);
    }

    count = 0;

    /* Handle negative numbers */
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

    /* Count digits */
    while (number)
    {
        number = number / 10;
        count++;
    }

    return (count + is_negative);
}

/* ==========================================================================
   String Length
   ========================================================================== */

size_t
ft_strlen(const char *s)
{
    size_t count;

    count = 0;

    while (s[count])
    {
        count++;
    }

    return (count);
}

/* ==========================================================================
   Character Search in String
   ========================================================================== */

char *
ft_strchr(const char *s, int c)
{
    while (*s)
    {
        if (*s == (char)c)
        {
            return ((char *)s);
        }
        s++;
    }

    /* Handle searching for null terminator */
    if (!*s && !c)
    {
        return ((char *)s);
    }

    return (NULL);
}
