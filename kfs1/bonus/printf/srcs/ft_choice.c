/* ************************************************************************** */
/*                                                                            */
/*   ft_choice.c - Format specifier handlers                                  */
/*                                                                            */
/*   Implements handlers for string, character, integer, pointer, and         */
/*   hexadecimal format specifiers.                                           */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_printf.h"
#include "screen.h"

/* ==========================================================================
   String Format Handler (%s)
   ========================================================================== */

void
ft_choice_s(t_sc *sc, va_list arg)
{
    char *s;

    s = va_arg(arg, char *);

    if (!s)
    {
        vga_puts("(null)", WHITE);
        sc->len = sc->len + 6;
    }
    else
    {
        ft_putstr(s);
        sc->len = sc->len + ft_strlen(s);
    }
}

/* ==========================================================================
   Character Format Handler (%c)
   ========================================================================== */

void
ft_choice_c(t_sc *sc, va_list arg)
{
    char c;

    c = va_arg(arg, int);
    ft_putchar(c);
    sc->len = sc->len + 1;
}

/* ==========================================================================
   Signed Integer Format Handler (%d, %i)
   ========================================================================== */

void
ft_choice_d_i(t_sc *sc, va_list arg)
{
    int i;

    i = va_arg(arg, int);
    ft_putnbr(i);
    sc->len = sc->len + ft_intlen(i);
}

/* ==========================================================================
   Pointer Format Handler (%p)
   ========================================================================== */

void
ft_choice_p(t_sc *sc, va_list arg)
{
    void *p;

    p = va_arg(arg, void *);

    if (!p)
    {
        vga_puts("0x0", WHITE);
        sc->len += 3;
    }
    else
    {
        vga_puts("0x", WHITE);
        sc->len += 2;
        ft_print_adress((unsigned long)p);
        sc->len = sc->len + ft_memlen((unsigned long)p, 16);
    }
}

/* ==========================================================================
   Hexadecimal Format Handler (%x, %X)
   ========================================================================== */

void
ft_choice_x(char c, t_sc *sc, va_list arg)
{
    unsigned int x;

    x = va_arg(arg, unsigned int);
    ft_print_hexa_x((unsigned long)x, c);

    if (x == 0 || x == (int)LONG_MIN)
    {
        sc->len = sc->len + ft_memlen((unsigned long)x, 16) + 1;
    }
    else
    {
        sc->len = sc->len + ft_memlen((unsigned long)x, 16);
    }
}
