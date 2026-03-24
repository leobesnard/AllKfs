/* ************************************************************************** */
/*                                                                            */
/*   other_fonctions.c - Basic output functions                               */
/*                                                                            */
/*   Implements character, string, and number output functions used by        */
/*   the printf implementation.                                               */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_printf.h"
#include "screen.h"

/* ==========================================================================
   Character Output
   ========================================================================== */

void
ft_putchar(char c)
{
    vga_putchar(c, WHITE);
}

/* ==========================================================================
   String Output
   ========================================================================== */

void
ft_putstr(char const *str)
{
    int idx;

    idx = 0;

    while (str[idx] != '\0')
    {
        ft_putchar(str[idx]);
        idx++;
    }
}

/* ==========================================================================
   Signed Number Output
   ========================================================================== */

void
ft_putnbr(int nb)
{
    /* Handle minimum integer value as special case */
    if (nb == -2147483648)
    {
        vga_puts("-2147483648", WHITE);
        return;
    }

    /* Handle single digit */
    if (nb >= 0 && nb < 10)
    {
        ft_putchar(nb + '0');
    }
    /* Handle negative numbers */
    else if (nb < 0)
    {
        ft_putchar('-');
        ft_putnbr(nb * (-1));
    }
    /* Handle multi-digit numbers */
    else
    {
        ft_putnbr(nb / 10);
        ft_putnbr(nb % 10);
    }
}

/* ==========================================================================
   Unsigned Number Output
   ========================================================================== */

void
ft_putnbr_u(unsigned int nb)
{
    if (nb < 10)
    {
        ft_putchar(nb + '0');
    }
    else
    {
        ft_putnbr(nb / 10);
        ft_putnbr(nb % 10);
    }
}

/* ==========================================================================
   Hexadecimal Output
   ========================================================================== */

void
ft_print_hexa_x(unsigned long nbr, char c)
{
    char    *hex;
    int     res[100];
    int     idx;

    /* Select case for hex digits */
    if (c == 'x')
    {
        hex = "0123456789abcdef";
    }
    else
    {
        hex = "0123456789ABCDEF";
    }

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
