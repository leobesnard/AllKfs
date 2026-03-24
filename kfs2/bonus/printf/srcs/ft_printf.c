/*
 * =============================================================================
 *                              KFS2 BONUS - FT_PRINTF MAIN
 * =============================================================================
 * Main kprintf implementation with format parsing
 * =============================================================================
 */

#include "../include/ft_printf.h"
#include "screen.h"

/* =============================================================================
 *                              INTERNAL FUNCTIONS
 * ============================================================================= */

/*
 * init_format_state - Initialize format state structure
 * @sc: Pointer to state structure to initialize
 */
static void init_format_state(t_sc *sc)
{
    sc->len = 0;
    sc->width = 0;
}

/*
 * ft_choice_u - Handle %u format specifier
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Prints unsigned decimal integer
 */
void ft_choice_u(t_sc *sc, va_list arg)
{
    int value;

    value = va_arg(arg, int);

    if (!value)
    {
        vga_puts("0", WHITE);
        sc->len += 1;
    }
    else
    {
        ft_putnbr_u((unsigned int)value);
        sc->len = sc->len + ft_memlen((unsigned int)value, 10);
    }
}

/*
 * ft_read_text - Read and output non-format characters
 * @format: Current position in format string
 * @sc: Format state
 *
 * Outputs characters until '%' or end of string
 * Returns: Updated format string pointer
 */
char const *ft_read_text(const char *format, t_sc *sc)
{
    char *next_format;

    next_format = ft_strchr(format, '%');

    if (next_format)
    {
        sc->width = next_format - format;
    }
    else
    {
        sc->width = ft_strlen(format);
    }

    vga_puts_n(format, WHITE, sc->width);
    sc->len = sc->len + sc->width;

    while (*format && *format != '%')
    {
        ++format;
    }

    return (format);
}

/*
 * ft_read_arg - Process format specifier and output value
 * @format: Current position in format string (after '%')
 * @sc: Format state
 * @arg: Variable argument list
 *
 * Returns: Updated format string pointer, or NULL on error
 */
char const *ft_read_arg(char const *format, t_sc *sc, va_list arg)
{
    if (*format == 's')
    {
        ft_choice_s(sc, arg);
    }
    else if (*format == 'c')
    {
        ft_choice_c(sc, arg);
    }
    else if (*format == 'i' || *format == 'd')
    {
        ft_choice_d_i(sc, arg);
    }
    else if (*format == 'X' || *format == 'x')
    {
        ft_choice_x(*format, sc, arg);
    }
    else if (*format == 'p')
    {
        ft_choice_p(sc, arg);
    }
    else if (*format == 'u')
    {
        ft_choice_u(sc, arg);
    }
    else if (*format == '%')
    {
        ft_choice_pourcent(sc);
    }
    else
    {
        return (NULL);
    }

    format++;
    return (format);
}

/* =============================================================================
 *                              MAIN PRINTF FUNCTION
 * ============================================================================= */

/*
 * kprintf - Kernel printf implementation
 * @format: Format string with optional specifiers
 *
 * Parses format string and outputs formatted text to VGA display
 * Returns: Number of characters printed, or -1 on error
 */
int kprintf(const char *format, ...)
{
    t_sc    sc;
    va_list arg;

    init_format_state(&sc);
    va_start(arg, format);

    while (*format)
    {
        if (*format == '%')
        {
            format = ft_read_arg(format + 1, &sc, arg);
        }
        else
        {
            format = ft_read_text(format, &sc);
        }

        if (!format)
        {
            va_end(arg);
            vga_puts("(null)", WHITE);
            return (-1);
        }
    }

    va_end(arg);
    return ((int)sc.len);
}
