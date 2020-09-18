BUILD ?= $(abspath build)
SHELL := /bin/bash

default: all

$(BUILD):
	mkdir -p $(BUILD)

all: m4-gc m0

m4-gc:
	mkdir -p build/m4-gc
	cd build/m4-gc && cmake -D TARGET_M4=ON -D TARGET_BOARD=grand_central_m4 ../../
	cd build/m4-gc && $(MAKE)

m0:
	mkdir -p build/m0
	cd build/m0 && cmake -D TARGET_M0=ON -D TARGET_BOARD=feather_m0 ../../
	cd build/m0 && $(MAKE)

clean:
	rm -rf $(BUILD)
