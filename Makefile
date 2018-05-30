BUILD ?= build

all: build

build:
	mkdir -p $(BUILD)
	cd build && cmake ../ && make

clean:
	rm -rf $(BUILD)

test: build
	build/examples/linux-mmap/example-linux-mmap-00 --truncate flash-00.bin
	build/examples/linux-mmap/example-linux-mmap-ff --truncate flash-ff.bin
	cd build && make test ARGS=-VV

test-00: build
	build/test/testall/testall-00 -VV

test-ff: build
	build/test/testall/testall-ff -VV

.PHONY: build clean
