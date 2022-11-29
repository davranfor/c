CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs -MMD -MP -O2
LDLIBS = -lm
TARGET = json
SRCDIR = src
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all release debug clean

all: $(TARGET)

release: $(TARGET)

debug: CFLAGS := $(filter-out -O2, $(CFLAGS)) -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)

