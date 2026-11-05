CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs)

SRCS := src/main.c src/game.c src/sdl2_platform.c

centurion: $(SRCS) src/platform.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ $(SRCS) $(SDL_LIBS)

clean:
	rm -f centurion

.PHONY: clean
