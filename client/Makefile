SRC	=	src/client.c

NAME	=	client

OBJ	=	$(SRC:.c=.o)
CFLAGS	=	-I$(PWD)/include -g -Wall -Wextra -Werror
CC	=	gcc

RM	=	rm -f

all:	$(OBJ)
	@$(CC) -o $(NAME) $(CFLAGS) $(OBJ)
	@echo "Client compiled successfully."

clean:
	@rm -f $(OBJ)
	@echo "Client Object cleaned."

fclean:	clean
	@rm -f $(NAME)
	@echo "Client cleaned."

re:	fclean all
	@echo "Client recompiled."