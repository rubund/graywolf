

DESTDIR := /
libdir := usr/local/lib

default:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

install:
	mkdir -p $(DESTDIR)$(libdir)/timberwolf/bin
	$(MAKE) -C src install


.PHONY: default clean install
