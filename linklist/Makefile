CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o linklist.o

all: linklist

main.o: linklist.h
linklist.o: linklist.h

linklist: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o linklist

clean:
	rm -f *.o linklist

