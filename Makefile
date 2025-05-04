# makefile for AddTraction
# see README for everything you may want to know ;)

BITMAP_PATH = /usr/share/games/addt/
INSTALL_PATH = /usr/bin
INSTALL_EXEC = cp

all:
	gcc -DBITMAP_PATH=\"$(BITMAP_PATH)\" -o addt -lSDL addt.c

install:
	mkdir -p $(BITMAP_PATH)
	$(INSTALL_EXEC) bmps/* $(BITMAP_PATH)
	$(INSTALL_EXEC) addt $(INSTALL_PATH)

clean:
	rm addt

uninstall:
	rm $(BITMAP_PATH)/*.bmp
	rmdir $(BITMAP_PATH)
	rm $(INSTALL_PATH)/addt
