/*
 * =============================================================================
 *                              KFS2 BONUS - OUTPUT FUNCTIONS
 * =============================================================================
 * Character, string, and number output functions for printf
 * =============================================================================
 */

#include "../include/ft_printf.h"
#include "screen.h"

/* =============================================================================
 *                              CHARACTER AND STRING OUTPUT
 * ============================================================================= */

/*
 * ft_putchar - Output a single character
 * @c: Character to output
 */
void ft_putchar(char c)
{
    vga_putchar(c, WHITE);
}

/*
 * ft_putstr - Output a null-terminated string
 * @str: String to output
 */
void ft_putstr(char const *str)
{
    int index;

    index = 0;
    while (str[index] != '\0')
    {
        ft_putchar(str[index]);
        index++;
    }
}

/* =============================================================================
 *                              INTEGER OUTPUT
 * ============================================================================= */

/*
 * ft_putnbr - Output a signed integer
 * @nb: Integer to output
 *
 * Handles INT_MIN special case and recursively prints digits
 */
void ft_putnbr(int nb)
{
    if (nb == -2147483648)
    {
        vga_puts("-2147483648", WHITE);
        return;
    }

    if (nb >= 0 && nb < 10)
    {
        ft_putchar(nb + '0');
    }
    else if (nb < 0)
    {
        ft_putchar('-');
        ft_putnbr(nb * (-1));
    }
    else
    {
        ft_putnbr(nb / 10);
        ft_putnbr(nb % 10);
    }
}

/*
 * ft_putnbr_u - Output an unsigned integer
 * @nb: Unsigned integer to output
 */
void ft_putnbr_u(unsigned int nb)
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

/* =============================================================================
 *                              HEXADECIMAL OUTPUT
 * ============================================================================= */

/*
 * ft_print_hexa_x - Output a number in hexadecimal format
 * @nbr: Number to convert
 * @c: Case specifier ('x' for lowercase, 'X' for uppercase)
 */
void ft_print_hexa_x(unsigned long nbr, char c)
{
    char    *hex_chars;
    int     digits[100];
    int     index;

    if (c == 'x')
    {
        hex_chars = "0123456789abcdef";
    }
    else
    {
        hex_chars = "0123456789ABCDEF";
    }

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
