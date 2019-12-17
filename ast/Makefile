
CC = gcc
CFLAGS = -I../hashmap
CFLAGS += -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
VPATH = ../hashmap
OBJECTS = main.o hashmap.o ast_eval.o ast_data.o ast.o

all: ast

main.o: ast.h
hashmap.o: hashmap.h
ast_eval.o:  ast_eval.h
ast_data.o: hashmap.h ast_eval.h ast_data.h
ast.o: ast_eval.h ast_data.h ast.h

ast: $(OBJECTS)
	$(CC) $(OBJECTS) -o ast -lm

clean:
	rm -f *.o ast

