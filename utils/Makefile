CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -g
LDLIBS = -lm

all: main.c utils.c wutils.c signed_maths.c
	$(CC) $(CFLAGS) main.c utils.c wutils.c signed_maths.c -o utils $(LDLIBS)

clean:
	rm -f utils test.txt
