CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs

all: main.c utils.c
	$(CC) $(CFLAGS) main.c utils.c -o utils -lm

clean:
	rm -f utils test.txt

