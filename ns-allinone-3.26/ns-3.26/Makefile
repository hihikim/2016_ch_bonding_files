# Makefile wrapper for waf

all:
	./waf

# free free to change this part to suit your requirements
configure:
	./waf configure --build-profile=optimized --enable-examples --enable-tests

build:
	./waf build

debug:
	./waf configure --build-profile=debug
	./waf

install:
	./waf install

clean:
	./waf clean

distclean:
	./waf distclean
