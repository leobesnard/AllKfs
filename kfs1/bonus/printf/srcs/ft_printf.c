/* ************************************************************************** */
/*                                                                            */
/*   ft_printf.c - Main printf implementation                                 */
/*                                                                            */
/*   Implements the kprintf function that parses format strings and           */
/*   dispatches to appropriate handlers for each format specifier.            */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_printf.h"
#include "screen.h"

/* ==========================================================================
   Internal Helper Functions
   ========================================================================== */

static void
initialized_structure(t_sc *sc)
{
    sc->len = 0;
    sc->width = 0;
}

/* ==========================================================================
   Unsigned Integer Format Handler
   ========================================================================== */

void
ft_choice_u(t_sc *sc, va_list arg)
{
    int u;

    u = va_arg(arg, int);

    if (!u)
    {
        vga_puts("0", WHITE);
        sc->len += 1;
    }
    else
    {
        ft_putnbr_u((unsigned int)u);
        sc->len = sc->len + ft_memlen((unsigned int)u, 10);
    }
}

/* ==========================================================================
   Text Reading (between format specifiers)
   ========================================================================== */

char const *
ft_read_text(const char *format, t_sc *sc)
{
    char *next;

    /* Find next format specifier */
    next = ft_strchr(format, '%');

    if (next)
    {
        sc->width = next - format;
    }
    else
    {
        sc->width = ft_strlen(format);
    }

    /* Output text segment */
    vga_puts_n(format, WHITE, sc->width);
    sc->len = sc->len + sc->width;

    /* Advance past printed text */
    while (*format && *format != '%')
    {
        ++format;
    }

    return (format);
}

/* ==========================================================================
   Format Argument Dispatcher
   ========================================================================== */

char const *
ft_read_arg(char const *format, t_sc *sc, va_list arg)
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

/* ==========================================================================
   Main Printf Function
   ========================================================================== */

int
kprintf(const char *format, ...)
{
    t_sc    sc;
    va_list arg;

    initialized_structure(&sc);
    va_start(arg, format);

    /* Process format string */
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

        /* Handle invalid format specifier */
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
