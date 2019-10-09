
CC = gcc
CFLAGS = -I../utils
CFLAGS += -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
VPATH = ../utils
OBJECTS = main.o file.o json.o

all: json

main.o: json.h
json.o: file.h json.h

json: $(OBJECTS)
	$(CC) $(OBJECTS) -o json

clean:
	rm -f *.o json

