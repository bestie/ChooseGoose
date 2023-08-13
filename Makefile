.PHONY: shell
.PHONY: clean
	
TOOLCHAIN_NAME=aveferrum/rg35xx-toolchain
WORKSPACE_DIR := $(shell pwd)/workspace

CONTAINER_NAME=$(shell docker ps -f "ancestor=$(TOOLCHAIN_NAME)" --format "{{.Names}}")
BOLD=$(shell tput bold)
NORM=$(shell tput sgr0)

xcomp: .build

test: $(WORKSPACE_DIR)/bin/x86/simple_menu
	$(WORKSPACE_DIR)/bin/x86/simple_menu

CC=gcc
CFLAGS = -Wall -framework Cocoa -framework CoreAudio -framework IOKit -framework CoreVideo -lSDLmain -lSDL -lSDL_ttf
INCLUDES = -I$(PREFIX)/include

$(WORKSPACE_DIR)/bin/x86/simple_menu:
	# $(CC) $^ -o $@ $(CFLAGS)
	# $(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	gcc -Wall -Iinclude -c "$(WORKSPACE_DIR)/src/main.c" -o "$(WORKSPACE_DIR)/obj/x86/main.o"
	gcc "$(WORKSPACE_DIR)/obj/x86/main.o" -o "$(WORKSPACE_DIR)/bin/x86/simple_menu -Wall -lSDL -lSDL_ttf

$(WORKSPACE_DIR)/bin/arm/simple_menu: .build
	docker run -t -v "$(WORKSPACE_DIR)":/root/workspace $(TOOLCHAIN_NAME) ./compile.sh

.build: Dockerfile
	$(info $(BOLD)Building $(TOOLCHAIN_NAME)...$(NORM))
	mkdir -p ./workspace
	docker build -t $(TOOLCHAIN_NAME) .
	touch .build

ifeq ($(CONTAINER_NAME),)
shell: .build
	echo "Found container ${CONTAINER_NAME}"
	$(info $(BOLD)Starting $(TOOLCHAIN_NAME)...$(NORM))
	docker run -it --rm -v "$(WORKSPACE_DIR)":/root/workspace $(TOOLCHAIN_NAME) /bin/bash
else
shell:
	echo "No container"
	$(info $(BOLD)Connecting to running $(TOOLCHAIN_NAME)...$(NORM))
	docker exec -it $(CONTAINER_NAME) /bin/bash  
endif

clean:
	$(info $(BOLD)Removing $(TOOLCHAIN_NAME)...$(NORM))
	docker rmi $(TOOLCHAIN_NAME)
	rm -f .build

push:
	$(info $(BOLD)Pushing $(TOOLCHAIN_NAME)...$(NORM))
	docker push $(TOOLCHAIN_NAME)
