
CC = gcc
CFLAGS = -I../hashmap
CFLAGS += -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -O2
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_mixer
VPATH = ../hashmap
OBJECTS = hashmap.o mapper.o bitmap.o sprite.o sound.o game.o load.o loop.o main.o menu.o play.o exit.o

all: game

hashmap.o: hashmap.h
mapper.o: hashmap.h mapper.h
bitmap.o: mapper.h bitmap.h
sprite.o: mapper.h sprite.h
sound.o: mapper.h sound.h
game.o: mapper.h bitmap.h sprite.h game.h
load.o: game.h load.h
loop.o: game.h task.h loop.h
main.o: game.h load.h loop.h
menu.o: game.h task.h bitmap.h
play.o: game.h task.h bitmap.h
exit.o: game.h task.h bitmap.h sprite.h sound.h

game: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o game $(LDFLAGS)

clean:
	rm -f *.o game

