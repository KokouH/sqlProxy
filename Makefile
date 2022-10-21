NAME = sqlProxy

SRC = main.cpp\
		acceptor.cpp\
		bridge.cpp
HDR = -I.
LINK = -lpthread

CC = g++

FLAGS = -Wall -Werror -Wextra

all: $(NAME)
	$(CC) $(FLAGS) $(HDR) $(SRC) $(LINK) -o $(NAME)

$(NAME): $(SRC)
	
clean:
	rm -rf $(NAME)

fclean: clean
	rm -rf $(NAME)
	
re: fclean all

.PHONY: all clean fclean re
