
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
OBJECTS = main.o game.o load.o play.o types.o handler.o

all: game

main.o: types.h game.h
game.o: types.h handler.h game.h
load.o: types.h load.h
play.o: types.h play.h
types.o: types.h
handler.o: types.h load.h play.h handler.h

game: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o game $(LDFLAGS)

clean:
	rm -f *.o game

