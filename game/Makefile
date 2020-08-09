
CC = gcc
CFLAGS = -I../hashmap
CFLAGS += -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -g
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
VPATH = ../hashmap
OBJECTS = hashmap.o bitmap.o game.o load.o loop.o main.o menu.o play.o exit.o

all: game

hashmap.o: hashmap.h
bitmap.o: hashmap.h bitmap.h
game.o: bitmap.h game.h
load.o: game.h load.h
loop.o: game.h task.h loop.h
main.o: game.h load.h loop.h
menu.o: game.h task.h bitmap.h
play.o: game.h task.h bitmap.h
exit.o: game.h task.h

game: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o game $(LDFLAGS)

clean:
	rm -f *.o game

