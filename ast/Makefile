
CC = gcc
CFLAGS = -I../utils -I../hashmap
CFLAGS += -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
VPATH = ../utils
VPATH += ../hashmap
OBJECTS = main.o utils.o hashmap.o ast_data.o ast_eval.o ast_heap.o ast.o

all: ast

main.o: utils.h ast.h
utils.o: utils.h
hashmap.o: hashmap.h
ast_data.o: hashmap.h ast_eval.h ast_data.h
ast_eval.o: ast_data.h ast_eval.h
ast_heap.o: ast_node.h ast_data.h ast_heap.h
ast.o: ast_node.h ast_data.h ast_eval.h ast_heap.h ast.h

ast: $(OBJECTS)
	$(CC) $(OBJECTS) -o ast -lm

clean:
	rm -f *.o ast

