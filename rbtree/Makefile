CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o rbtree.o

all: rbtree

main.o: rbtree.h
rbtree.o: rbtree.h

rbtree: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o rbtree

clean:
	rm -f *.o rbtree

