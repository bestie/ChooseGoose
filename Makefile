# Project meta-Makefile for running all project tasks, Docker builds, compilation, dependencies, deployment, etc.

PROJECT_NAME=ChooseGoose
PROJECT_SHORT=choose_goose
IMAGE_TAG=rg35xx_choose_goose:latest
WORKSPACE_DIR := $(shell pwd)/workspace
BIN_DIR=$(WORKSPACE_DIR)/bin
DESTINATION_DIR=RG/$(PROJECT_NAME)
RG_APPS=/mnt/mmc/Roms/APPS
RG_DESTINATION=$(RG_APPS)/$(PROJECT_NAME)

.PHONY: cross_compile
cross_compile: docker_build
	docker run --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash --login -c make
	mkdir -p $(DESTINATION_DIR)
	cp $(BIN_DIR)/* $(DESTINATION_DIR)

# Compile and run the app natively
test:
	cd workspace && make -f Makefile.local && ls ../example/roms | binx64/$(PROJECT_SHORT)

.PHONY: push
push: cross_compile
	cp ./workspace/bin/$(PROJECT_SHORT) RG/${PROJECT_NAME}
	cp -r $(WORKSPACE_DIR)/assets $(DESTINATION_DIR)
	cp -r libyaml-armv7/lib $(DESTINATION_DIR)
	adb push --sync RG/* $(RG_APPS)
	adb shell ls -l $(RG_DESTINATION)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) $(DESTINATION_DIR)

.PHONY: clean_rg
clean_rg:
	adb shell rm -rf /mnt/mmc/Roms/APPS/ChooseGoos*

.PHONY: shell
shell:
	docker run --interactive --tty --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash

.PHONY: docker_build
docker_build:
	docker build --tag $(IMAGE_TAG) .

.PHONY: clean
docker_clean:
	docker rmi $(IMAGE_TAG)
