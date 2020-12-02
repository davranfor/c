CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o dynarray.o

all: dynarray

main.o: dynarray.h
dynarray.o: dynarray.h

dynarray: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o dynarray

clean:
	rm -f *.o dynarray

