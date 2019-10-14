
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o vector.o

all: vector

main.o: vector.h
vector.o: vector.h

vector: $(OBJECTS)
	$(CC) $(OBJECTS) -o vector

clean:
	rm -f *.o vector

