
CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -g
OBJECTS = main.o gui.o

all: sdl2

main.o: gui.h
gui.o: gui.h

sdl2: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o sdl2 `sdl2-config --cflags --libs` -lSDL2_ttf

clean:
	rm -f *.o sdl2

