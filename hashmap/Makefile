CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o hashmap.o

all: hashmap

main.o: hashmap.h
hashmap.o: hashmap.h

hashmap: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o hashmap

clean:
	rm -f *.o hashmap

