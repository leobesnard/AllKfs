/* ************************************************************************** */
/*                                                                            */
/*   other_fonction_3.c - Memory and address utility functions                */
/*                                                                            */
/*   Implements memory address length calculation, percent sign output,       */
/*   and hexadecimal address printing.                                        */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_printf.h"

/* ==========================================================================
   Memory Address Digit Count
   ========================================================================== */

int
ft_memlen(unsigned long addr, unsigned int base)
{
    int             count;
    unsigned long   number;
    int             is_negative;

    /* Handle negative values (though address is unsigned) */
    if (addr < 0)
    {
        number = -addr;
        is_negative = 1;
    }
    else
    {
        number = addr;
        is_negative = 0;
    }

    /* Count digits in specified base */
    count = 0;
    while (number)
    {
        number /= base;
        count++;
    }

    return (count + is_negative);
}

/* ==========================================================================
   Percent Sign Output (%%)
   ========================================================================== */

void
ft_choice_pourcent(t_sc *sc)
{
    ft_putchar('%');
    sc->len = sc->len + 1;
}

/* ==========================================================================
   Pointer Address Output
   ========================================================================== */

void
ft_print_adress(unsigned long nbr)
{
    char    *hex;
    int     res[100];
    int     idx;

    hex = "0123456789abcdef";

    /* Convert to hex (stored in reverse) */
    idx = 0;
    while (nbr >= 16)
    {
        res[idx] = hex[nbr % 16];
        nbr = nbr / 16;
        idx++;
    }
    res[idx] = hex[nbr];

    /* Output in correct order */
    while (idx >= 0)
    {
        ft_putchar(res[idx]);
        idx--;
    }
}
