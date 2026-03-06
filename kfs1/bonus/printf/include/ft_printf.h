#ifndef FT_PRINTF_H
# define FT_PRINTF_H
// # include <stdarg.h>
// # include <stdint.h>
// # include <stdbool.h>
// # include <stddef.h>
// # include <stdio.h>
// # include <unistd.h>
// # include <stdlib.h>
// # include <stdarg.h>
// # include <stdlib.h>

typedef char** va_list;
typedef unsigned int size_t;
// Unsigned integer types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// Signed integer types
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
#define LONG_MIN (-2147483648)
#define NULL ((void *)0)

#define VA_SIZE(type) ((sizeof(type) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1))
#define va_start(ap, last) (*ap = (char*)(&last + 1))
#define va_arg(ap, type) (*(type *)((*ap += VA_SIZE(type)) - VA_SIZE(type)))
#define va_end(ap) (*ap = NULL)

typedef struct s_sc
{
	size_t	len;
	size_t	width;
}				t_sc;

void	ft_choice_pourcent(t_sc *sc);
void	ft_putchar(char c);
void	ft_putstr(char const *str);
void	ft_putnbr(int nb);
void	ft_print_hexa_x(unsigned long nbr, char c);
void	ft_print_adress(unsigned long nbr);
void	ft_putnbr_u(unsigned int nb);
size_t	ft_strlen(const char *s);
char	*ft_strchr(const char *s, int c);
void	ft_choice_s(t_sc *sc, va_list arg);
void	ft_choice_c(t_sc *sc, va_list arg);
void	ft_choice_d_i(t_sc *sc, va_list arg);
void	ft_choice_p(t_sc *sc, va_list arg);
void	ft_choice_x(char c, t_sc *sc, va_list arg);
int		ft_memlen(unsigned long addr, unsigned int base);
int		ft_intlen(int nb);
int		kprintf(const char *format, ...);

#endif
