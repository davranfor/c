
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o timer.o

all: timer

main.o: main.c timer.h
	$(CC) $(CFLAGS) -c main.c

timer.o: timer.c timer.h
	$(CC) $(CFLAGS) -c timer.c

timer: $(OBJECTS)
	$(CC) $(OBJECTS) -o timer -lpthread

clean:
	rm -f *.o timer

