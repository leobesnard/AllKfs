#include "../include/ft_printf.h"

int	ft_memlen(unsigned long addr, unsigned int base)
{
	int				i;
	unsigned long	number;
	int				negatif;

	if (addr < 0)
	{
		number = -addr;
		negatif = 1;
	}
	else
	{
		number = addr;
		negatif = 0;
	}
	i = 0;
	while (number)
	{
		number /= base;
		i++;
	}
	return (i + negatif);
}

void	ft_choice_pourcent(t_sc *sc)
{
	ft_putchar('%');
	sc->len = sc->len + 1;
}

void	ft_print_adress(unsigned long nbr)
{
	char	*hex;
	int		res[100];
	int		i;		

	hex = "0123456789abcdef";
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
