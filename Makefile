CC		 := gcc
CFLAGS	 := -Wall -Wextra -g3 $(shell pkg-config --cflags glib-2.0)
LDFLAGS	 := $(shell pkg-config --libs glib-2.0) -lncursesw  -lm
EXEC	 := main
TERM 	 := mainTerminal
SRC		 := projet/main.c
OBJ		 := $(SRC:.c=.o)

all: $(EXEC)

term : $(TERM)

$(EXEC): $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)

$(TERM): $(OBJ)
		@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

re: mrproper $(EXEC)

clean:
	@rm -rf projet/*.o

mrproper: clean
	@rm -rf $(EXEC)
