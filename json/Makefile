CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -MMD -MP -O2
SOURCE = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(SOURCE))
LDLIBS = -lm

.PHONY: all release debug clean

all: json

release: all

debug: CFLAGS := $(filter-out -O2, $(CFLAGS)) -DDEBUG -g
debug: all

json: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o json

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

$(shell mkdir -p obj)

clean:
	rm -rf obj
	rm -f json

