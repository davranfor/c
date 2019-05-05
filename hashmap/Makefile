
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o hashmap.o

all: hashmap

main.o: main.c hashmap.h
	$(CC) $(CFLAGS) -c main.c

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c

hashmap: $(OBJECTS)
	$(CC) $(OBJECTS) -o hashmap

clean:
	rm -f *.o hashmap

