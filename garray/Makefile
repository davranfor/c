CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o garray.o

all: garray

main.o: garray.h
garray.o: garray.h

garray: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o garray

clean:
	rm -f *.o garray

