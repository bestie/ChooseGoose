PLATFORM ?= $(shell uname -s)
ARCH ?= $(shell uname -m)
LIBC ?= glibc
PREFIX ?= "/usr"

BUILD_DIR = build/$(PLATFORM)-$(ARCH)-$(LIBC)

ifeq ($(PLATFORM), Darwin)
	# if /usr/local/include exists, otherwise check for /opt/homebrew/include
	ifneq ("$(wildcard /usr/local/include)", "")
		PREFIX = /usr/local
	else
		PREFIX = /opt/homebrew
	endif

	MORE_LDS = -framework CoreFoundation -framework Cocoa
endif

CC ?= $(CROSS_COMPILE)gcc
CXX ?= $(CROSS_COMPILE)g++
LD ?= $(CROSS_COMPILE)ld
AR ?= $(CROSS_COMPILE)ar
AS ?= $(CROSS_COMPILE)as
LDFLAGS = -L$(PREFIX)/lib -lSDL -lSDLmain -lSDL_image -lSDL_ttf $(MORE_LDS)
CFLAGS = -g -std=c11 -Wall -Wextra -Werror
INCLUDES = -Iinclude -Ibuild -I$(PREFIX)/include

PROJECT_NAME=ChooseGoose
PROJECT_SHORT=choosegoose
PROJECT_ROOT := $(shell pwd)
DESTINATION_DIR=RG/$(PROJECT_NAME)
RG_APPS=/mnt/mmc/Roms/APPS
RG_DESTINATION=$(RG_APPS)/$(PROJECT_NAME)

OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
TARGET = $(BIN_DIR)/$(PROJECT_SHORT)
SRC_DIR = src

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all
all: compile_flags.txt docker-compile docker-compile-rg35xx

.PHONY: goose
goose: $(TARGET)

.PHONY: clean
clean:
	rm -rf build/*

compile_flags.txt: Makefile
	# split C_FLAGS into lines
	echo $(CFLAGS) | tr ' ' '\n' > $@
	echo $(INCLUDES) | tr ' ' '\n' >> $@

### Embedded background image #################################################

BACKGROUND_IMAGE = assets/bg.png
COMPILED_BG_IMAGE = build/background_image.c

$(COMPILED_BG_IMAGE): $(BACKGROUND_IMAGE)
	cp $(BACKGROUND_IMAGE) build/default_background_image
	cd build && xxd -i default_background_image $(PROJECT_ROOT)/$@

### Embedded font compilation ##################################################

FONT_DOWNLOAD = build/dejavu-fonts-ttf-2.37.tar.bz2
TTF_FONT_FILE = build/fonts/font.ttf
COMPILED_FONT = build/font.c

.PHONY: font
font: $(COMPILED_FONT)

$(COMPILED_FONT): $(TTF_FONT_FILE)
	cd build/fonts && xxd -i default_font $(PROJECT_ROOT)/$@

$(TTF_FONT_FILE): $(FONT_DOWNLOAD)
	mkdir -p build/fonts
	tar --directory build/fonts -xf $(FONT_DOWNLOAD)
	cp build/fonts/dejavu-fonts-ttf-2.37/ttf/DejaVuSansMono.ttf build/fonts/default_font

$(FONT_DOWNLOAD):
	mkdir -p build
	wget https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-fonts-ttf-2.37.tar.bz2 \
		--directory-prefix=build

### Compile and link ##########################################################

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(BUILD_DIR) $(BIN_DIR) $(COMPILED_FONT) $(COMPILED_BG_IMAGE) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

### Docker #####################################################################

DOCKER_TAG ?= choosegoose:latest
DOCKER_BUILD_CACHE_FILE = build/.docker-build
RG_EXECUTABLE = build/Linux-arm-uclibcgnueabi/bin/choosegoose

.PHONY: docker-build
docker-build: $(BUILD_DIR) $(DOCKER_BUILD_CACHE_FILE)

$(DOCKER_BUILD_CACHE_FILE): $(BUILD_DIR) Dockerfile Makefile $(SOURCES)
	docker build --platform linux/amd64 --tag $(DOCKER_TAG) . && touch $(DOCKER_BUILD_CACHE_FILE)

.PHONY: docker-compile
docker-compile: $(SOURCES) $(DOCKER_BUILD_CACHE_FILE)
	docker run --platform linux/amd64 --rm --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) make goose

.PHONY: docker-compile-rg35xx
docker-compile-rg35xx: $(SOURCES) $(DOCKER_BUILD_CACHE_FILE)
	docker run --platform linux/amd64 --rm --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) bash -c "source cross_compilation_env.sh && make goose"
	cp -r RG35XX build/
	cp $(RG_EXECUTABLE) build/RG35XX/demos/APPS/ChooseGoose/

.PHONY: docker-shell
docker-shell: $(DOCKER_BUILD_CACHE_FILE)
	docker run --platform linux/amd64 --rm --interactive --tty --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) /bin/bash

.PHONY: docker-clean
docker_clean:
	docker rmi $(DOCKER_TAG) && rm $(DOCKER_BUILD_CACHE_FILE)

### RG35XX #####################################################################

RG_APPS = /mnt/mmc/Roms/APPS
RG_ROMS = /mnt/SDCARD/Roms
RG_INSTALL_DIR = $(RG_APPS)/$(PROJECT_NAME)

.PHONY: rg-install
rg-install:
	adb shell mkdir -p $(RG_INSTALL_DIR)
	cp -r RG35XX build/
	cp $(RG_EXECUTABLE) build/RG35XX/demos/APPS/ChooseGoose/
	adb push --sync build/RG35XX/demos/APPS/* $(RG_APPS)
	adb push --sync build/RG35XX/demos/ROMS/* $(RG_ROMS)
	adb shell ls -l $(RG_APPS)
	adb shell ls -l $(RG_DESTINATION)

.PHONY: rg-logs
rg-logs:
	adb shell touch $(RG_INSTALL_DIR)/log
	echo " ~~~ Open the app 👉 🎮"
	adb shell busybox tail -f $(RG_DESTINATION)/log
	# Get the directory listing and filter for the name of the TV Show

.PHONY: adb-shell
adb-shell: adb-start
	adb shell

.PHONY: clean-rg
clean_rg: adb-start
	adb start-server
	adb usb
	adb shell rm -rf /mnt/mmc/Roms/APPS/ChooseGoos*

.PHONY: adb-start
	adb start-server
	adb usb

