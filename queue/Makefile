
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o queue.o

all: queue

main.o: queue.h
queue.o: queue.h

queue: $(OBJECTS)
	$(CC) $(OBJECTS) -o queue

clean:
	rm -f *.o queue

