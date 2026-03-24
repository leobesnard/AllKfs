/*
 * =============================================================================
 *                              KFS2 BONUS - FORMAT SPECIFIER HANDLERS
 * =============================================================================
 * Handlers for various printf format specifiers
 * =============================================================================
 */

#include "../include/ft_printf.h"
#include "screen.h"

/* =============================================================================
 *                              STRING AND CHARACTER HANDLERS
 * ============================================================================= */

/*
 * ft_choice_s - Handle %s format specifier
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Prints a null-terminated string, or "(null)" if NULL
 */
void ft_choice_s(t_sc *sc, va_list arg)
{
    char *str;

    str = va_arg(arg, char *);

    if (!str)
    {
        vga_puts("(null)", WHITE);
        sc->len = sc->len + 6;
    }
    else
    {
        ft_putstr(str);
        sc->len = sc->len + ft_strlen(str);
    }
}

/*
 * ft_choice_c - Handle %c format specifier
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Prints a single character
 */
void ft_choice_c(t_sc *sc, va_list arg)
{
    char ch;

    ch = va_arg(arg, int);
    ft_putchar(ch);
    sc->len = sc->len + 1;
}

/* =============================================================================
 *                              INTEGER HANDLERS
 * ============================================================================= */

/*
 * ft_choice_d_i - Handle %d and %i format specifiers
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Prints a signed decimal integer
 */
void ft_choice_d_i(t_sc *sc, va_list arg)
{
    int value;

    value = va_arg(arg, int);
    ft_putnbr(value);
    sc->len = sc->len + ft_intlen(value);
}

/* =============================================================================
 *                              POINTER AND HEX HANDLERS
 * ============================================================================= */

/*
 * ft_choice_p - Handle %p format specifier
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Prints a pointer address in hexadecimal with "0x" prefix
 */
void ft_choice_p(t_sc *sc, va_list arg)
{
    void *ptr;

    ptr = va_arg(arg, void *);

    if (!ptr)
    {
        vga_puts("0x0", WHITE);
        sc->len += 3;
    }
    else
    {
        vga_puts("0x", WHITE);
        sc->len += 2;
        ft_print_adress((unsigned long)ptr);
        sc->len = sc->len + ft_memlen((unsigned long)ptr, 16);
    }
}

/*
 * ft_choice_x - Handle %x and %X format specifiers
 * @c: Format character ('x' for lowercase, 'X' for uppercase)
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Prints an unsigned integer in hexadecimal format
 */
void ft_choice_x(char c, t_sc *sc, va_list arg)
{
    unsigned int hex_value;

    hex_value = va_arg(arg, unsigned int);
    ft_print_hexa_x((unsigned long)hex_value, c);

    if (hex_value == 0 || hex_value == (int)LONG_MIN)
    {
        sc->len = sc->len + ft_memlen((unsigned long)hex_value, 16) + 1;
    }
    else
    {
        sc->len = sc->len + ft_memlen((unsigned long)hex_value, 16);
    }
}
