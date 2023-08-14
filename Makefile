PROJECT_NAME=RG35XX_SimpleMenu
IMAGE_TAG=rg35xx_simplemenu:latest
WORKSPACE_DIR := $(shell pwd)/workspace
BINARY=simple_menu

.PHONY: xcompile
xcompile: docker_build
	docker run -it --rm -v "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) ./compile.sh
	cp -r $(WORKSPACE_DIR)/bin RG/SimpleMenu

.PHONY: push
push: xcomp
	cp ./workspace/bin/simple_menu RG/SimpleMenu/
	adb push RG/* /mnt/mmc/Roms/APPS

.PHONY: shell
shell:
	docker run --interactive --tty --rm --volume "$(WORKSPACE_DIR)":/root/workspace $(IMAGE_TAG) /bin/bash

.PHONY: docker_build
docker_build:
	docker build --tag $(IMAGE_TAG) .

.PHONY: clean
docker_clean:
	docker rmi $(IMAGE_TAG)
