CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -MMD -MP -O2
LDLIBS = -lm
TARGET = json
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(SOURCES))

.PHONY: all release debug clean

all: $(TARGET)

release: $(TARGET)

debug: CFLAGS := $(filter-out -O2, $(CFLAGS)) -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(TARGET)

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	@mkdir -p $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf obj
	rm -f $(TARGET)

