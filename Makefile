.PHONY: all build sqush clean

all: build

build:
	./build.sh

sqush:
	./enroot.sh -n efa-test -f "${PWD}/Dockerfile"

clean:
	rm -rf build/
