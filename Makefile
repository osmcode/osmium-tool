
all:
	mkdir -p build && cd build && cmake .. && make

clean:
	mkdir -p build && cd build && cmake .. && make clean

distclean:
	rm -fr build

deb:
	debuild -I -us -uc

