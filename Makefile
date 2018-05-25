BUILD ?= build

all: build

build:
	mkdir -p $(BUILD)
	cd build && cmake ../ && make

clean:
	rm -rf $(BUILD)

test: build
	build/examples/linux-mmap/example-linux-mmap-00
	build/examples/linux-mmap/example-linux-mmap-ff

.PHONY: build clean
