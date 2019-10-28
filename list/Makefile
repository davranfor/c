
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o list.o

all: list

main.o: list.h
list.o: list.h

list: $(OBJECTS)
	$(CC) $(OBJECTS) -o list

clean:
	rm -f *.o list

