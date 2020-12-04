CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o klist.o

all: klist

main.o: klist.h
klist.o: klist.h

klist: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o klist

clean:
	rm -f *.o klist

