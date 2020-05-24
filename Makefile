NAME = ssu_crontab
SRCS = main.c ssu_crontab.c
OBJS = $(SRCS:.c=.o)

CC = gcc

.PHONY = all clean fclean re

all: $(NAME)

$(NAME) : $(OBJS)

.c.o: ssu_crontab.h
	$(CC) -c -g $< -I.

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
