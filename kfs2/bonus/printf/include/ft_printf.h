/*
 * =============================================================================
 *                              KFS2 BONUS - FT_PRINTF HEADER
 * =============================================================================
 * Custom printf implementation for kernel use
 * Provides formatted output without standard library dependencies
 * =============================================================================
 */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

/* =============================================================================
 *                              TYPE DEFINITIONS
 * ============================================================================= */

/* Variable argument list type */
typedef char** va_list;

/* Standard size type */
typedef unsigned int size_t;

/* Unsigned integer types */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/* Signed integer types */
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

/* =============================================================================
 *                              CONSTANTS
 * ============================================================================= */

#define LONG_MIN (-2147483648)
#define NULL ((void *)0)

/* =============================================================================
 *                              VARIADIC ARGUMENT MACROS
 * ============================================================================= */

/* Calculate aligned size for a type (4-byte alignment) */
#define VA_SIZE(type) ((sizeof(type) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1))

/* Initialize va_list to point after the last named parameter */
#define va_start(ap, last) (*ap = (char*)(&last + 1))

/* Retrieve next argument and advance pointer */
#define va_arg(ap, type) (*(type *)((*ap += VA_SIZE(type)) - VA_SIZE(type)))

/* Clean up va_list (set to NULL) */
#define va_end(ap) (*ap = NULL)

/* =============================================================================
 *                              FORMAT STATE STRUCTURE
 * ============================================================================= */

/*
 * t_sc - Format state tracking structure
 * @len: Total characters written so far
 * @width: Width of current field
 */
typedef struct s_sc
{
    size_t  len;
    size_t  width;
}               t_sc;

/* =============================================================================
 *                              FORMAT SPECIFIER HANDLERS
 * ============================================================================= */

void    ft_choice_pourcent(t_sc *sc);
void    ft_choice_s(t_sc *sc, va_list arg);
void    ft_choice_c(t_sc *sc, va_list arg);
void    ft_choice_d_i(t_sc *sc, va_list arg);
void    ft_choice_p(t_sc *sc, va_list arg);
void    ft_choice_x(char c, t_sc *sc, va_list arg);

/* =============================================================================
 *                              OUTPUT FUNCTIONS
 * ============================================================================= */

void    ft_putchar(char c);
void    ft_putstr(char const *str);
void    ft_putnbr(int nb);
void    ft_putnbr_u(unsigned int nb);
void    ft_print_hexa_x(unsigned long nbr, char c);
void    ft_print_adress(unsigned long nbr);

/* =============================================================================
 *                              UTILITY FUNCTIONS
 * ============================================================================= */

size_t  ft_strlen(const char *s);
char    *ft_strchr(const char *s, int c);
int     ft_memlen(unsigned long addr, unsigned int base);
int     ft_intlen(int nb);

/* =============================================================================
 *                              MAIN PRINTF FUNCTION
 * ============================================================================= */

/*
 * kprintf - Kernel printf implementation
 * @format: Format string with optional specifiers
 * @...: Variable arguments matching format specifiers
 *
 * Supported specifiers:
 *   %s - String
 *   %c - Character
 *   %d, %i - Signed decimal integer
 *   %u - Unsigned decimal integer
 *   %x - Lowercase hexadecimal
 *   %X - Uppercase hexadecimal
 *   %p - Pointer address
 *   %% - Literal percent sign
 *
 * Returns: Number of characters printed, or -1 on error
 */
int     kprintf(const char *format, ...);

#endif
