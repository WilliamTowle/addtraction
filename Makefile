# makefile for AddTraction
# see README for Andre's gameplay and build information

.PHONY: default
default: all

USE_CUSTOM_SDL=n


# Configuration for SDL v1.2?

ifeq (${USE_SDL_VERSION},1.2)
ifneq (${USE_CUSTOM_SDL},n)
DIR_TOOLCHAIN?=${CURDIR}/toolchain
TRACE:=$(shell echo "DIR_TOOLCHAIN ${DIR_TOOLCHAIN}" 1>&2)
include Makefile.SDL
SDL_CONFIG=${DIR_TOOLCHAIN}/bin/sdl-config
else
SDL_CONFIG=$(shell which sdl-config)
endif
endif


BITMAP_PATH = /usr/share/games/addt/
INSTALL_PATH = /usr/bin
INSTALL_EXEC = cp

CC = gcc
CFLAGS = -DBITMAP_PATH=\"$(BITMAP_PATH)\"
SDL_CFLAGS?=$(shell [ -r "${SDL_CONFIG}" ] && ${SDL_CONFIG} --cflags)
SDL_LIBS?=$(shell [ -r "${SDL_CONFIG}" ] && ${SDL_CONFIG} --libs)


.PHONY: all
ifneq (${USE_CUSTOM_SDL},n)
all:: toolchain-sdl
endif
all:: addt

addt: addt.c
	${CC} ${CFLAGS} $< ${SDL_CFLAGS} -o $@ ${SDL_LIBS}

.PHONY: install
install:
	mkdir -p $(BITMAP_PATH)
	$(INSTALL_EXEC) bmps/* $(BITMAP_PATH)
	$(INSTALL_EXEC) addt $(INSTALL_PATH)

.PHONY: clean
clean::
	rm addt

.PHONY: uninstall
uninstall:
	rm $(BITMAP_PATH)/*.bmp
	rmdir $(BITMAP_PATH)
	rm $(INSTALL_PATH)/addt
