CC = g++
PKG_CONFIG = pkg-config

BASE_CFLAGS=-std=c++11 -Wall -Wextra -O3 `$(PKG_CONFIG) --cflags Qt5DBus mpv`
BASE_LDFLAGS=`$(PKG_CONFIG) --libs Qt5DBus`

kde-night-color.so: kde-night-color.c
	$(CC) kde-night-color.c -o kde-night-color.so $(BASE_CFLAGS) $(CFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -shared -fPIC

install: kde-night-color.so
	mkdir -p $(HOME)/.config/mpv/scripts
	cp kde-night-color.so -t $(HOME)/.config/mpv/scripts

clean:
	rm kde-night-color.so
