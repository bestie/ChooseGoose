PROJECT_NAME=RG35XX_SimpleMenu
IMAGE_TAG=rg35xx_simplemenu:latest
WORKSPACE_DIR := $(shell pwd)/workspace
BIN_DIR=$(WORKSPACE_DIR)/bin
DESTINATION_DIR=RG/SimpleMenu/bin

.PHONY: cross_compile
cross_compile: docker_build
	docker run --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash --login -c make
	mkdir -p $(DESTINATION_DIR)
	cp $(BIN_DIR)/* $(DESTINATION_DIR)

.PHONY: push
push: xcomp
	cp ./workspace/bin/simple_menu RG/SimpleMenu/
	adb push RG/* /mnt/mmc/Roms/APPS

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) $(DESTINATION_DIR)

.PHONY: shell
shell:
	docker run --interactive --tty --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash

.PHONY: docker_build
docker_build:
	docker build --tag $(IMAGE_TAG) .

.PHONY: clean
docker_clean:
	docker rmi $(IMAGE_TAG)
