/*
 * =============================================================================
 *                              KFS2 BONUS - MEMORY AND ADDRESS FUNCTIONS
 * =============================================================================
 * Memory length calculation and address printing functions
 * =============================================================================
 */

#include "../include/ft_printf.h"

/* =============================================================================
 *                              MEMORY LENGTH FUNCTION
 * ============================================================================= */

/*
 * ft_memlen - Calculate number of digits in a number for a given base
 * @addr: Number to measure
 * @base: Numeric base (e.g., 10 for decimal, 16 for hex)
 *
 * Returns: Number of digits needed to represent the number in the given base
 */
int ft_memlen(unsigned long addr, unsigned int base)
{
    int             digit_count;
    unsigned long   number;
    int             is_negative;

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

    digit_count = 0;
    while (number)
    {
        number /= base;
        digit_count++;
    }

    return (digit_count + is_negative);
}

/* =============================================================================
 *                              SPECIAL FORMAT HANDLERS
 * ============================================================================= */

/*
 * ft_choice_pourcent - Handle %% format specifier
 * @sc: Format state
 *
 * Outputs a literal percent sign
 */
void ft_choice_pourcent(t_sc *sc)
{
    ft_putchar('%');
    sc->len = sc->len + 1;
}

/*
 * ft_print_adress - Output a pointer address in hexadecimal
 * @nbr: Address value to print
 *
 * Outputs lowercase hexadecimal without "0x" prefix
 */
void ft_print_adress(unsigned long nbr)
{
    char    *hex_chars;
    int     digits[100];
    int     index;

    hex_chars = "0123456789abcdef";
    index = 0;

    while (nbr >= 16)
    {
        digits[index] = hex_chars[nbr % 16];
        nbr = nbr / 16;
        index++;
    }
    digits[index] = hex_chars[nbr];

    /* Output digits in reverse order (most significant first) */
    while (index >= 0)
    {
        ft_putchar(digits[index]);
        index--;
    }
}
