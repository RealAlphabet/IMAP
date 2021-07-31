##
## RealAlphabet, 2021
## Makefile
##

NAME        =   imap
SRC         =   $(shell find src -name "*.c")
OBJ         =   $(SRC:.c=.o)

CFLAGS      =   -Wall                   \
                -Werror                 \
                -Wno-pointer-sign       \
                -Wno-unused-variable    \
                -I include              \
                -Llibs                  \
                -lmbedtls               \
                -lmbedx509              \
                -lmbedcrypto            \
                -g

all:    $(NAME)

$(NAME):$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(CFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:		fclean all