
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o binmap.o

all: binmap

main.o: binmap.h
binmap.o: binmap.h

binmap: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o binmap

clean:
	rm -f *.o binmap

