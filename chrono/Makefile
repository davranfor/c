
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o chrono.o

all: chrono

main.o: chrono.h
chrono.o: chrono.h

chrono: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o chrono

clean:
	rm -f *.o chrono

