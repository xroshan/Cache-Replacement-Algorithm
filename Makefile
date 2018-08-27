#Makefile

CC=gcc
CFLAGS = --std=c11 -O0 -DSF_VISIBILITY -fvisibility=hidden -fno-strict-aliasing

SRCDIR = src
INCDIR = include
BUILDDIR = build

TARGET = bin/cacher

SOURCES = $(wildcard $(SRCDIR)/*.c)

OBJECTS= $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))

INCLUDE = -I $(INCDIR)

all: $(TARGET)
rebuild: all

debug: CFLAGS += -g
debug: all

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) 

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	-rm -r $(BUILDDIR) $(TARGET)

.PHONY: clean
