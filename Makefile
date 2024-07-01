ifdef CROSS_COMPILE
else
	PLATFORM := $(shell uname -s)
	ARCH := $(shell uname -m)
	LIBC := $(shell ldd --version 2>&1 | head -n 1 | awk '{print $$NF}')
	PREFIX ?= "/usr"
endif

BUILD_DIR = build/$(PLATFORM)-$(ARCH)-$(LIBC)

ifeq ($(PLATFORM), Darwin)
    PREFIX ?= /usr/local
		MORE_LDS = -framework CoreFoundation -framework Cocoa
else ifeq ($(PLATFORM)), Linux)
endif

CC ?= $(CROSS_COMPILE)gcc
CXX ?= $(CROSS_COMPILE)g++
LD ?= $(CROSS_COMPILE)ld
AR ?= $(CROSS_COMPILE)ar
AS ?= $(CROSS_COMPILE)as
LDFLAGS ?= -L$(PREFIX)/lib -lSDL -lSDLmain -lSDL_image -lSDL_ttf $(MORE_LDS)
CFLAGS = -g -Wall -lSDLmain -lSDL -lSDL_ttf -lSDL_image
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

.PHONY: goose
goose: $(TARGET)

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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(BIN_DIR) $(COMPILED_FONT) $(COMPILED_BG_IMAGE) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf build/*

.PHONY: clean_rg
clean_rg:
	adb shell rm -rf /mnt/mmc/Roms/APPS/ChooseGoos*

### Docker #####################################################################

DOCKER_TAG ?= choosegoose:latest
DOCKER_BUILD_CACHE_FILE = build/.docker-build

.PHONY: docker-build
docker-build: $(DOCKER_BUILD_CACHE_FILE)

$(DOCKER_BUILD_CACHE_FILE): Dockerfile Makefile $(SOURCES)
	docker build --tag $(DOCKER_TAG) . && touch $(DOCKER_BUILD_CACHE_FILE)

.PHONY: docker-compile
docker-compile: docker-build
	docker run --rm --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) make goose

.PHONY: docker-compile-rg35xx
docker-compile-rg35xx: docker-build
	docker run --rm --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) bash -c "source cross_compilation_env.sh && make goose"

.PHONY: docker-shell
shell: $(DOCKER_BUILD_CACHE_FILE)
	docker run --rm --interactive --tty --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) /bin/bash

.PHONY: docker-clean
docker_clean:
	docker rmi $(DOCKER_TAG) && rm $(DOCKER_BUILD_CACHE_FILE)
