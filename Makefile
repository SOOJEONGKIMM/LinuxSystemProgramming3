
NAME1 = ssu_crontab
SRCS1 = ssu_crontab.c
OBJS1 = $(SRCS1:.c=.o)

NAME2 = ssu_rsync
SRCS2 = ssu_rsync.c
OBJS2 = $(SRCS2:.c=.o)

NAME3 = ssu_crond
SRCS3 = ssu_crond.c
OBJS3 = $(SRCS3:.c=.o)

all: $(NAME1) $(NAME2) $(NAME3)

CC = gcc

.PHONY = all clean fclean re


$(NAME1) : $(OBJS1) 
	$(CC) -o $@ $< 
$(NAME2) : $(OBJS2)
	$(CC) -o $@ $<
$(NAME3) : $(OBJS3)
	$(CC) -o $@ $<

.c.o: ssu_crontab.h ssu_rsync.h ssu_crond.h


clean:
	rm -rf $(OBJS1) ${OBJS2) $(OBJS3)

fclean: clean
	rm -rf $(NAME1) $(NAME2) $(NAME3)

re: fclean all
