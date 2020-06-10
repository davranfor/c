
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o ringlist.o

all: ringlist

main.o: ringlist.h
ringlist.o: ringlist.h

ringlist: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o ringlist

clean:
	rm -f *.o ringlist

