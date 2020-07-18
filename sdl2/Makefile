
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
OBJECTS = main.o sdl.o

all: sdl2

main.o: sdl.h
gui.o: sdl.h

sdl2: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o sdl2 `sdl2-config --cflags --libs` -lSDL2_ttf

clean:
	rm -f *.o sdl2

