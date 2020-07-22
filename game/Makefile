
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
OBJECTS = main.o load.o play.o game.o

all: mcvspf

main.o: types.h game.h
load.o: types.h load.h
play.o: types.h play.h
game.o: types.h load.h game.h

mcvspf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o game $(LDFLAGS)

clean:
	rm -f *.o game

