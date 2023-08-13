# Use the provided environment variables
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
AS = $(CROSS_COMPILE)as

CFLAGS = -Wall
INCLUDES = -I$(PREFIX)/include
LDFLAGS = -L$(PREFIX)/lib -lSDL -lSDL_ttf

TARGET = simple_menu
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	mkdir -p $(@D)
	/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc obj/main.o -L/opt/miyoo/lib -lSDL -lSDL_ttf -o bin/simple_menu
	# $(CC) $(LDFLAGS) $^ -o $@ $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc -Wall -I/opt/miyoo/include -c src/main.c -o obj/main.o
	# $(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

print:
	echo "BINDIR = ${BINDIR}"
	echo "TARGET = ${TARGET}"
	echo "SRCDIR = ${SRCDIR}"
	echo "OBJDIR = ${OBJDIR}"
	echo "BINDIR = ${BINDIR}"

.PHONY: all clean
