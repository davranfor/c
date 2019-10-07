
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o file.o json.o

all: json

main.o: main.c file.h json.h
	$(CC) $(CFLAGS) -c main.c

file.o: file.c file.h
	$(CC) $(CFLAGS) -c file.c

json.o: json.c json.h
	$(CC) $(CFLAGS) -c json.c

json: $(OBJECTS)
	$(CC) $(OBJECTS) -o json

clean:
	rm -f *.o json

