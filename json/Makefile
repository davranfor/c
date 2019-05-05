
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o json.o

all: json

main.o: main.c json.h
	$(CC) $(CFLAGS) -c main.c

json.o: json.c json.h
	$(CC) $(CFLAGS) -c json.c

json: $(OBJECTS)
	$(CC) $(OBJECTS) -o json

clean:
	rm -f *.o json

