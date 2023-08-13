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
/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc -g -Wall -I/opt/miyoo/include -c src/main.c -o obj/main.o
# Link
/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc obj/main.o -g -L/opt/miyoo/lib -lSDL -lSDL_ttf -o bin/simple_menu
# Static link
# /opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-gcc obj/main.o -g -L/opt/miyoo/lib /opt/miyoo/lib/libSDL.a /opt/miyoo/lib/libSDL_ttf.a -o bin/simple_menu

