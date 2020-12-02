CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o skiplist.o

all: skiplist

main.o: skiplist.h
skiplist.o: skiplist.h

skiplist: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o skiplist

clean:
	rm -f *.o skiplist

