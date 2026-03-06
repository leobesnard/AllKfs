NAME	=	libftprintf.a

CC	=	gcc -I ../includes/

FLAG	= -m32 -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs

SRC	=	srcs/ft_printf.c	srcs/other_fonctions.c	srcs/ft_choice.c	srcs/other_fonctions_2.c	srcs/other_fonction_3.c 
			
OBJ	=	$(SRC:.c=.o)

all:	$(NAME)

$(NAME):	$(OBJ)
	@ar rcs $@ $^
	@echo "$@ created"
	@ranlib $@
	@echo "$@ indexed"

%.o: %.c
	@$(CC) $(FLAG) -o $@ -c $<

clean:
	@rm -f $(OBJ)
	@echo "OBJS deleted"

fclean:	clean
	@rm -f $(NAME)
	@echo "$(NAME) deleted"

re:		fclean all

.PHONY: all, clean, fclean, re
