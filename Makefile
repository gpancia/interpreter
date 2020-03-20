.PHONY: all clean

all:
	rm -rf build
	make -C src

clean:
	rm -rf build
