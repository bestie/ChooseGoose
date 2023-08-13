#!/bin/bash

echo "running compile.sh"

mkdir -p bin
mkdir -p obj

#ifeq ($(shell uname -s), Darwin) # macOS
#    CFLAGS = -Wall -framework Cocoa -framework CoreAudio -framework IOKit -framework CoreVideo -lSDLmain -lSDL -lSDL_ttf
#else ifeq ($(shell uname -s), Linux)
#    CFLAGS = -Wall -lSDLmain -lSDL -lSDL_ttf
#endif

# Compile
/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc -Wall -I/opt/miyoo/include -c src/main.c -o obj/main.o
# Link
/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc obj/main.o -L/opt/miyoo/lib -lSDL -lSDL_ttf -o bin/simple_menu

