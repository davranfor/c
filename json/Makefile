CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o json.o json_format.o json_schema.o json_utils.o
LDLIBS = -lm

all: json

json.o: json.h
json_format.o: json_format.h
json_schema.o: json.h json_format.h json_schema.h
json_utils.o: json.h json_utils.h
main.o: json.h json_schema.h json_utils.h

json: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o json $(LDLIBS)

clean:
	rm -f *.o json

