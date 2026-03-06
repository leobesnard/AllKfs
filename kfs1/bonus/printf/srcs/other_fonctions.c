#include "../include/ft_printf.h"
#include "screen.h"

void	ft_putchar(char c)
{
	// write(1, &c, 1);
	// print_char(c, WHITE);W
	// char outCou[] = {c, '\0'};
	// print_str(outCou, WHITE);
	print_char(c, WHITE);
}

void	ft_putstr(char const *str)
{
	int	i;

	i = 0;
	while (str[i] != '\0')
	{
		ft_putchar(str[i]);
		i++;
	}
}

void	ft_putnbr(int nb)
{
	if (nb == -2147483648)
	{
		print_str("-2147483648", WHITE);
		// write(1, "-2147483648", 11);
		return ;
	}
	if (nb >= 0 && nb < 10)
		ft_putchar(nb + '0');
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

void	ft_putnbr_u(unsigned int nb)
{
	if (nb < 10)
		ft_putchar(nb + '0');
	else
	{
		ft_putnbr(nb / 10);
		ft_putnbr(nb % 10);
	}
}

void	ft_print_hexa_x(unsigned long nbr, char c)
{
	char	*hex;
	int		res[100];
	int		i;

	if (c == 'x')
		hex = "0123456789abcdef";
	else
		hex = "0123456789ABCDEF";
	i = 0;
	while (nbr >= 16)
	{
		res[i] = hex[nbr % 16];
		nbr = nbr / 16;
		i++;
	}
	res[i] = hex[nbr];
	while (i >= 0)
	{
		ft_putchar(res[i]);
		i--;
	}
}
