
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o skiplist.o

all: skiplist

main.o: main.c skiplist.h
	$(CC) $(CFLAGS) -c main.c

skiplist.o: skiplist.c skiplist.h
	$(CC) $(CFLAGS) -c skiplist.c

skiplist: $(OBJECTS)
	$(CC) $(OBJECTS) -o skiplist

clean:
	rm -f *.o skiplist

