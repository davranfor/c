
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o skiplist.o

all: skiplist

main.o: skiplist.h
skiplist.o: skiplist.h

skiplist: $(OBJECTS)
	$(CC) $(OBJECTS) -o skiplist

clean:
	rm -f *.o skiplist

