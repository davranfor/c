
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs

all: main.c utils.c
	$(CC) $(CFLAGS) main.c utils.c -o utils

clean:
	rm -f utils

