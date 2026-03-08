.PHONY: clean build

all: build

clean:
	rm -rf build

build: clean
	cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake -B build
	cmake --build build
	make -C build
