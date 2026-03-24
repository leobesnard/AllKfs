/* ************************************************************************** */
/*                                                                            */
/*   ft_printf.h - Printf implementation for kernel                           */
/*                                                                            */
/*   Provides formatted output functionality similar to standard printf,      */
/*   adapted for freestanding kernel environment without standard library.    */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

/* ==========================================================================
   Type Definitions (no standard library available)
   ========================================================================== */

/* Variadic argument list type */
typedef char **va_list;

/* Standard size type */
typedef unsigned int size_t;

/* Unsigned integer types */
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long      uint64_t;

/* Signed integer types */
typedef char                    int8_t;
typedef short                   int16_t;
typedef int                     int32_t;
typedef long long               int64_t;

/* ==========================================================================
   Constants
   ========================================================================== */

#define LONG_MIN    (-2147483648)
#define NULL        ((void *)0)

/* ==========================================================================
   Variadic Argument Macros
   ========================================================================== */

/* Calculate aligned size for type on stack */
#define VA_SIZE(type) \
    ((sizeof(type) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1))

/* Initialize variadic argument list */
#define va_start(ap, last) \
    (*ap = (char *)(&last + 1))

/* Get next argument from variadic list */
#define va_arg(ap, type) \
    (*(type *)((*ap += VA_SIZE(type)) - VA_SIZE(type)))

/* Clean up variadic argument list */
#define va_end(ap) \
    (*ap = NULL)

/* ==========================================================================
   Format State Structure
   ========================================================================== */

typedef struct s_sc
{
    size_t  len;        /* Total characters written */
    size_t  width;      /* Current field width */
}   t_sc;

/* ==========================================================================
   Format Specifier Handlers
   ========================================================================== */

void    ft_choice_pourcent(t_sc *sc);
void    ft_choice_s(t_sc *sc, va_list arg);
void    ft_choice_c(t_sc *sc, va_list arg);
void    ft_choice_d_i(t_sc *sc, va_list arg);
void    ft_choice_p(t_sc *sc, va_list arg);
void    ft_choice_x(char c, t_sc *sc, va_list arg);

/* ==========================================================================
   Output Functions
   ========================================================================== */

void    ft_putchar(char c);
void    ft_putstr(char const *str);
void    ft_putnbr(int nb);
void    ft_putnbr_u(unsigned int nb);
void    ft_print_hexa_x(unsigned long nbr, char c);
void    ft_print_adress(unsigned long nbr);

/* ==========================================================================
   Utility Functions
   ========================================================================== */

size_t  ft_strlen(const char *s);
char    *ft_strchr(const char *s, int c);
int     ft_memlen(unsigned long addr, unsigned int base);
int     ft_intlen(int nb);

/* ==========================================================================
   Main Printf Function
   ========================================================================== */

int     kprintf(const char *format, ...);

#endif
