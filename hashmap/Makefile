
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o hashmap.o

all: hashmap

main.o: hashmap.h
hashmap.o: hashmap.h

hashmap: $(OBJECTS)
	$(CC) $(OBJECTS) -o hashmap

clean:
	rm -f *.o hashmap

