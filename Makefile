CC = gcc

ifeq ($(shell uname -s), Darwin) # macOS
    CFLAGS = -Wall -framework Cocoa -framework CoreAudio -framework IOKit -framework CoreVideo -lSDLmain -lSDL -lSDL_ttf
else ifeq ($(shell uname -s), Linux)
    CFLAGS = -Wall -lSDLmain -lSDL -lSDL_ttf
endif

INCLUDES = -Iinclude
TARGET = simple_menu
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

docker_build:
	docker build -t rgtools .

shell: docker_build
	docker run -it rgtools /bin/bash

build: docker_build
	docker run -t rgtools make

clean:
	rm -rf $(OBJDIR) $(BINDIR)

run: all
	./$(BINDIR)/$(TARGET)

.PHONY: all
