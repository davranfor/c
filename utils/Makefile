CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lm

all: main.c utils.c wutils.c
	$(CC) $(CFLAGS) main.c utils.c wutils.c -o utils $(LDLIBS)

clean:
	rm -f utils test.txt
