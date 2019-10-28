
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o stack.o

all: stack

main.o: stack.h
stack.o: stack.h

stack: $(OBJECTS)
	$(CC) $(OBJECTS) -o stack

clean:
	rm -f *.o stack

