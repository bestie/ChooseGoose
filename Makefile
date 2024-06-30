# Project meta-Makefile for running all project tasks, Docker builds, compilation, dependencies, deployment, etc.

PLATFORM := $(shell uname -s)
ARCH := $(shell uname -m)
LIBC_VERSION := $(shell ldd --version 2>&1 | head -n 1 | awk '{print $$NF}')
BUILD_DIR := build/$(PLATFORM)-$(ARCH)-libc$(LIBC_VERSION)

ifeq ($(PLATFORM), Darwin) # macOS
    CFLAGS = -g -Wall -framework Cocoa -framework CoreAudio -framework IOKit -framework CoreVideo -lSDLmain -lSDL -lSDL_ttf -lSDL_image -lyaml
	CC ?= clang
else ifeq ($(PLATFORM)), Linux)
    CFLAGS = -g -Wall -lSDLmain -lSDL -lSDL_ttf -lSDL_image -lyaml
	CC ?= gcc
endif

PROJECT_NAME=ChooseGoose
PROJECT_SHORT=choosegoose
PROJECT_ROOT := $(shell pwd)
IMAGE_TAG=rg35xx_choosegoose:latest
WORKSPACE_DIR=$(PROJECT_ROOT)/workspace
BIN_DIR=$(WORKSPACE_DIR)/bin
EXECUTABLE=$(BIN_DIR)/$(PROJECT_SHORT)
DESTINATION_DIR=RG/$(PROJECT_NAME)
RG_APPS=/mnt/mmc/Roms/APPS
RG_DESTINATION=$(RG_APPS)/$(PROJECT_NAME)

INCLUDES = -Iworkspace/include -Ibuild
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
TARGET = $(BIN_DIR)/$(PROJECT_SHORT)
SRC_DIR = workspace/src

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# $(EXECUTABLE): $(WORKSPACE_DIR)/src/* $(COMPILED_BG_IMAGE) $(COMPILED_FONT)
# 	docker run --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash --login -c make
# 	mkdir -p $(DESTINATION_DIR)
# 	cp $(BIN_DIR)/* $(DESTINATION_DIR)
# 	@echo "✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅"


.PHONY: rg_executable
rg_executable: $(EXECUTABLE)

.PHONY: deploy
deploy: clean clean_rg $(EXECUTABLE) push
	@echo "🚢"

.PHONY: push
push:
	cp $(EXECUTABLE) RG/${PROJECT_NAME}
	cp $(WORKSPACE_DIR)/config.yaml RG/${PROJECT_NAME}
	mkdir -p $(DESTINATION_DIR)
	mkdir -p $(DESTINATION_DIR)/lib
	mkdir -p $(DESTINATION_DIR)/assets
	cp $(WORKSPACE_DIR)/assets/bg_no_sky.png  $(DESTINATION_DIR)/assets
	cp $(WORKSPACE_DIR)/assets/font.ttf       $(DESTINATION_DIR)/assets
	cp -r libyaml-armv7/lib/libyaml-0.so.2 $(DESTINATION_DIR)/lib
	adb push --sync RG/* $(RG_APPS)
	adb push --sync RG/* $(RG_APPS)
	adb shell ls -l $(RG_DESTINATION)

# Tail the app's log on the device
.PHONY: tail_rg
tail_rg:
	adb shell touch $(RG_DESTINATION)/events.log
	adb shell touch $(RG_DESTINATION)/goose.log
	echo " ~~~ Open the app 👉 🎮"
	adb shell busybox tail -f $(RG_DESTINATION)/*.log

# Compile and run the app natively
.PHONY: test
test: $(TARGET)
	cat RG/MD_rom_list.txt | sort | $(TARGET)

.PHONY: bin
bin: $(TARGET)

### Embedded background image #################################################

BACKGROUND_IMAGE = assets/bg.png
COMPILED_BG_IMAGE = build/background_image.c

$(COMPILED_BG_IMAGE): $(BACKGROUND_IMAGE)
	cp $(BACKGROUND_IMAGE) build/default_background_image
	cd build && xxd -i default_background_image $(PROJECT_ROOT)/$@

### Embedded font compilation ##################################################

FONT_DOWNLOAD = assets/dejavu-fonts-ttf-2.37.tar.bz2 
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
	mkdir -p assets
	wget https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-fonts-ttf-2.37.tar.bz2 \
		--directory-prefix=assets

###############################################################################

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(BIN_DIR) $(COMPILED_FONT) $(COMPILED_BG_IMAGE) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -rf build/*

.PHONY: clean_rg
clean_rg:
	adb shell rm -rf /mnt/mmc/Roms/APPS/ChooseGoos*

.PHONY: shell
shell: docker_build
	docker run --interactive --tty --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash

.PHONY: docker_build
docker_build: .docker_build

.docker_build: Dockerfile
	docker build --tag $(IMAGE_TAG) . && touch .docker_build

.PHONY: docker_clean
docker_clean:
	docker rmi $(IMAGE_TAG)
