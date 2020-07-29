
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
OBJECTS = main.o game.o init.o load.o play.o loop.o

all: game

main.o: game.h init.h
game.o: game.h
init.o: game.h loop.h init.h
load.o: game.h load.h
play.o: game.h play.h
loop.o: game.h load.h play.h loop.h

game: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o game $(LDFLAGS)

clean:
	rm -f *.o game

