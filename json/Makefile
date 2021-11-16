CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o json.o

all: json

main.o: json.h
json.o: json.h

json: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o json

clean:
	rm -f *.o json

