CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o knode.o

all: knode

main.o: knode.h
knode.o: knode.h

knode: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o knode

clean:
	rm -f *.o knode

