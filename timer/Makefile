
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o timer.o

all: timer

main.o: timer.h
timer.o: timer.h

timer: $(OBJECTS)
	$(CC) $(OBJECTS) -o timer -lpthread

clean:
	rm -f *.o timer

