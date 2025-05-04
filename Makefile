# makefile for AddTraction
# see README for Andre's gameplay and build information

.PHONY: default
default: all

BITMAP_PATH = /usr/share/games/addt/
INSTALL_PATH = /usr/bin
INSTALL_EXEC = cp

CC = gcc
SDL_LIBS = -lSDL

.PHONY: all
all: addt

addt: addt.c
	${CC} -DBITMAP_PATH=\"$(BITMAP_PATH)\" -o $@ ${SDL_LIBS} $<

.PHONY: install
install:
	mkdir -p $(BITMAP_PATH)
	$(INSTALL_EXEC) bmps/* $(BITMAP_PATH)
	$(INSTALL_EXEC) addt $(INSTALL_PATH)

.PHONY: clean
clean:
	rm addt

.PHONY: uninstall
uninstall:
	rm $(BITMAP_PATH)/*.bmp
	rmdir $(BITMAP_PATH)
	rm $(INSTALL_PATH)/addt
