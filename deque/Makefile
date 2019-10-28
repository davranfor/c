
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o deque.o

all: deque

main.o: deque.h
deque.o: deque.h

deque: $(OBJECTS)
	$(CC) $(OBJECTS) -o deque

clean:
	rm -f *.o deque

