CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o splaytree.o

all: splaytree

main.o: splaytree.h
splaytree.o: splaytree.h

splaytree: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o splaytree

clean:
	rm -f *.o splaytree

