# Project meta-Makefile for running all project tasks, Docker builds, compilation, dependencies, deployment, etc.

PROJECT_NAME=ChooseGoose
PROJECT_SHORT=choose_goose
IMAGE_TAG=rg35xx_choose_goose:latest
WORKSPACE_DIR := $(shell pwd)/workspace
BIN_DIR=$(WORKSPACE_DIR)/bin
EXECUTABLE=$(BIN_DIR)/$(PROJECT_SHORT)
DESTINATION_DIR=RG/$(PROJECT_NAME)
RG_APPS=/mnt/mmc/Roms/APPS
RG_DESTINATION=$(RG_APPS)/$(PROJECT_NAME)

NATIVE_EXECUTABLE=$(WORKSPACE_DIR)/binx64/$(PROJECT_SHORT)

$(EXECUTABLE): $(WORKSPACE_DIR)/src/*
	docker run --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash --login -c make
	mkdir -p $(DESTINATION_DIR)
	cp $(BIN_DIR)/* $(DESTINATION_DIR)
	@echo "✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅"

.PHONY: rg_executable
rg_executable: $(EXECUTABLE)

.PHONY: deploy
deploy: clean clean_rg $(EXECUTABLE) push
	@echo "🚢"

.PHONY: push
push:
	cp $(EXECUTABLE) RG/${PROJECT_NAME}
	cp $(EXECUTABLE) RG/${PROJECT_NAME}
	cp $(WORKSPACE_DIR)/config.yaml RG/${PROJECT_NAME}
	mkdir -p $(DESTINATION_DIR)
	cp $(WORKSPACE_DIR)/assets/bg_no_sky.png  $(DESTINATION_DIR)/assets
	cp $(WORKSPACE_DIR)/assets/font.ttf       $(DESTINATION_DIR)/assets
	cp -r libyaml-armv7/lib $(DESTINATION_DIR)
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
test: $(NATIVE_EXECUTABLE)
	cd workspace && ls ../example/roms | sort | binx64/$(PROJECT_SHORT)

$(NATIVE_EXECUTABLE): $(WORKSPACE_DIR)/src/*
	cd workspace && make -f Makefile.local clean all

.PHONY: clean
clean:
	cd workspace && make clean
	cd workspace && make -f Makefile.local clean

.PHONY: clean_rg
clean_rg:
	adb shell rm -rf /mnt/mmc/Roms/APPS/ChooseGoos*

.PHONY: shell
shell:
	docker run --interactive --tty --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash

.PHONY: docker_build
docker_build: .docker_build

.docker_build: Dockerfile
	docker build --tag $(IMAGE_TAG) . && touch .docker_build

.PHONY: docker_clean
docker_clean:
	docker rmi $(IMAGE_TAG)
