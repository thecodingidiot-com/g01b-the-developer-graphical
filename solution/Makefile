CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -D_REENTRANT -I libtci
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf
SRCS    = main.c load.c game.c render.c font.c
OBJS    = $(SRCS:.c=.o)
LIBS    = libtci/libtci.a
NAME    = game

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS) $(LIBS)
	$(CC) $(OBJS) $(LIBS) $(LDFLAGS) -o $(NAME)

$(LIBS):
	$(MAKE) -C libtci

%.o: %.c game.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C libtci clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C libtci fclean
	rm -f $(NAME)

re: fclean all
