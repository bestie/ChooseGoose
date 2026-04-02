OS ?= $(shell uname -s)
ARCH ?= $(shell uname -m)

ifeq ($(OS), Linux)
	LIBC ?= "glibc"
  PLATFORM := $(OS)-$(ARCH)-$(LIBC)
else
  PLATFORM := $(OS)-$(ARCH)
endif

BUILD_DIR = build/$(PLATFORM)

ifeq ($(OS), Darwin)
	PREFIX=/opt/homebrew

	VENDOR_PREFIX := vendor/build

	CFLAGS += -g -std=c11 -Wall \
						-Iinclude -Ibuild \
						-I$(VENDOR_PREFIX)/include \
						-I$(VENDOR_PREFIX)/include/SDL

	LDFLAGS += \
		$(VENDOR_PREFIX)/lib/libfreetype.a \
		$(VENDOR_PREFIX)/lib/libpng.a \
		$(VENDOR_PREFIX)/lib/libSDL.a \
		$(VENDOR_PREFIX)/lib/libSDLmain.a \
		$(VENDOR_PREFIX)/lib/libSDL_image.a \
		$(VENDOR_PREFIX)/obj/libSDL_ttf.o \
		$(shell pkg-config --libs libbrotlidec zlib) \
		-framework Cocoa \
		-framework CoreFoundation \
		-framework AudioUnit \
		-framework CoreAudio \
		-framework OpenGL \
		-framework IOKit \
		-framework Carbon \
		-framework ApplicationServices \
    -Wl,-rpath,@executable_path/../Frameworks

	DEPENDENCY_INSTALL_CMD = ./static-compile-deps.sh
else # Linux
	PREFIX=/usr

  SDL_PKGS = sdl SDL_image SDL_ttf

	CFLAGS += -g -std=c11 -Wall \
						-Iinclude -Ibuild \
						$(shell pkg-config --cflags $(SDL_PKGS))

	LDFLAGS += $(shell pkg-config --libs $(SDL_PKGS))

	DEPENDENCY_INSTALL_CMD = apt-get install -y libcriterion-dev libsdl1.2-dev libsdl-ttf2.0-dev libsdl-image1.2-dev
endif

CC ?= $(CROSS_COMPILE)gcc
CXX ?= $(CROSS_COMPILE)g++
LD ?= $(CROSS_COMPILE)ld
AR ?= $(CROSS_COMPILE)ar
AS ?= $(CROSS_COMPILE)as


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

HOME ?=$(shell echo $$HOME)
INSTALL_LOCATION ?= $(HOME)/bin/$(PROJECT_SHORT)

# Default target compiles the library for the native platform without Docker
.PHONY: choosegoose
choosegoose: $(TARGET)

.PHONY: install
install: $(TARGET)
	cp $(TARGET) $(INSTALL_LOCATION)

.PHONY: goose
goose: $(TARGET)

.PHONY: all
all: compile_flags.txt docker-compile docker-compile-rg35xx rg-demos

.PHONY: demo
demo: $(TARGET)
	ls -1 | ./$(TARGET) --title "It's a demo" --background-image=DEFAULT --log-file /dev/stderr

.PHONY: app-demo
app-demo: $(TARGET)
	ls -1 /Applications | ./$(TARGET) --title "App Launcher" --hide-file-extensions=true --background-image=DEFAULT --log-file /dev/stderr | xargs -I{} open -a "{}"

# Default clean task does not remove the font download because it's annoying
# to download after every clean when it doesn't actaully change.
.PHONY: clean
clean:
	find build -mindepth 1 \
		-not -path "$(FONT_DIR)" \
		-not -path "$(FONT_DIR)/*" \
		-exec rm -rf {} +

compile_flags.txt: Makefile
	echo $(CFLAGS) | tr ' ' '\n' > $@

.PHONY: install-dependencies
install-dependencies:
	bash -l -c "$(DEPENDENCY_INSTALL_CMD)"

.PHONY: echo-build-dir
echo-build-dir:
	@echo $(BUILD_DIR)

.PHONY: echo-bin-path
echo-bin-path:
	@echo $(TARGET)

### Embedded background image #################################################

BACKGROUND_IMAGE = assets/bg.png
COMPILED_BG_IMAGE = build/background_image.c

$(COMPILED_BG_IMAGE): $(BACKGROUND_IMAGE)
	cp $(BACKGROUND_IMAGE) build/default_background_image
	cd build && xxd -i default_background_image $(PROJECT_ROOT)/$@

### Embedded font compilation ##################################################

FONT_DIR = build/font
FONT_DOWNLOAD = $(FONT_DIR)/dejavu-fonts-ttf-2.37.tar.bz2
# No file extension because xxd uses the full file name for the variable name
# when it generates the C code to embed the file
TTF_FONT_FILE = $(FONT_DIR)/default_font
COMPILED_FONT = build/font.c

.PHONY: clean-font
clean-font:
	rm -rf $(FONT_DIR) $(COMPILED_FONT)

.PHONY: font
font: $(COMPILED_FONT)

$(COMPILED_FONT): $(TTF_FONT_FILE)
	cd $(FONT_DIR) && xxd -i $(notdir $(TTF_FONT_FILE)) > $(PROJECT_ROOT)/$@

$(TTF_FONT_FILE): $(FONT_DOWNLOAD)
	tar --directory $(FONT_DIR) -xf $(FONT_DOWNLOAD)
	cp $(FONT_DIR)/dejavu-fonts-ttf-2.37/ttf/DejaVuSansMono.ttf $(TTF_FONT_FILE)

.PHONY: font-download
font_download: $(FONT_DOWNLOAD)
$(FONT_DOWNLOAD):
	mkdir -p $(FONT_DIR)
	wget https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-fonts-ttf-2.37.tar.bz2 \
		--directory-prefix=$(FONT_DIR)

### Compile and link ##########################################################

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECTS): $(COMPILED_FONT) $(COMPILED_BG_IMAGE)

$(TARGET): $(BUILD_DIR) $(BIN_DIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

### Tests #####################################################################

TEST_BUILD_DIR = $(BUILD_DIR)/tests
TEST_OBJ_DIR = $(TEST_BUILD_DIR)/obj
TEST_SRC_DIR = tests
TEST_SOURCES = $(wildcard $(TEST_SRC_DIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_SRC_DIR)/%.c=$(TEST_OBJ_DIR)/%.o)
TEST_EXECUTABLE = $(TEST_BUILD_DIR)/$(PROJECT_SHORT)_tests

.PHONY: test
test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE) --jobs=1

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	mkdir -p $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -I$(PREFIX)/include/criterion -Isrc -c $< -o $@

$(TEST_EXECUTABLE): $(TARGET) $(TEST_OBJECTS) $(TEST_SOURCES)
	echo "Building tests"
	echo "objects in $(OBJECTS)"
	$(CC) $(TEST_OBJECTS) $(OBJECTS) -o $@ -lcriterion $(LDFLAGS)

### Docker #####################################################################

DOCKER_TAG ?= choosegoose:latest
DOCKER_BUILD_CACHE_FILE = build/.docker-build

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

.PHONY: docker-shell
docker-shell: $(DOCKER_BUILD_CACHE_FILE)
	docker run --platform linux/amd64 --rm --interactive --tty --volume "$(PROJECT_ROOT)/build:/root/choosegoose/build" $(DOCKER_TAG) /bin/bash

.PHONY: docker-clean
docker_clean:
	docker rmi $(DOCKER_TAG) && rm $(DOCKER_BUILD_CACHE_FILE)

### RG35XX #####################################################################

RG_ROMS = /mnt/mmc/Roms
RG_INSTALL_DIR = $(RG_APPS)/$(PROJECT_NAME)
RG_EXECUTABLE = build/Linux-arm-uclibcgnueabi/bin/choosegoose

.PHONY: rg-demos
rg-demos:
	cp -r RG35XX build/
	cp $(RG_EXECUTABLE) build/RG35XX/demos/Roms/APPS/ChooseGoose/
	RG35XX/demos/videos.sh build/RG35XX/demos/Roms/TV/

build/RG35XX/demos/Roms/APPS/ChooseGoose/choosegoose:
	cp -r RG35XX build/
	cp $(RG_EXECUTABLE) build/RG35XX/demos/Roms/APPS/ChooseGoose/

.PHONY: rg-install
rg-install: rg-demos
	adb shell mkdir -p $(RG_INSTALL_DIR)
	adb push --sync build/RG35XX/demos/Roms/* $(RG_ROMS)
	adb shell ls -l $(RG_APPS)
	adb shell ls -l $(RG_DESTINATION)

.PHONY: rg-logs
rg-logs:
	adb shell touch $(RG_INSTALL_DIR)/log
	echo " ~~~ Open the app 👉 🎮"
	adb shell busybox tail -f $(RG_DESTINATION)/log

.PHONY: adb-shell
adb-shell: adb-start
	adb shell

.PHONY: clean-rg
clean-rg:
	adb shell rm -rf $(RG_ROMS)/APPS/ChooseGame.sh
	adb shell rm -rf $(RG_ROMS)/APPS/ChooseLibrary.sh
	adb shell rm -rf $(RG_ROMS)/APPS/ChooseGoose

.PHONY: adb-start
adb-start:
	adb start-server
	adb usb

.PHONY: tarball
tarball: build/choosegoose.tar.gz

build/choosegoose.tar.gz: $(BUILD_DIR) $(SOURCES) $(TEST_SOURCES) include/*.h
	git ls-files | grep -E -v "\.(png|mkv)$$" | tar -czf $@ -T -

