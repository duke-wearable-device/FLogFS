BUILD ?= build

all: build

build:
	mkdir -p $(BUILD)
	cd build && cmake ../ && make

clean:
	rm -rf $(BUILD)

test: build
	build/examples/linux-mmap/example-linux-mmap-00 --truncate
	build/examples/linux-mmap/example-linux-mmap-ff --truncate
	cd build && make test ARGS=-VV

.PHONY: build clean
