CC = g++
PKG_CONFIG = pkg-config

INSTALL := install
MKDIR := mkdir
RMDIR := rmdir
LN := ln
RM := rm

CFLAGS += -std=c++11 -Wall -Wextra -O2 `$(PKG_CONFIG) --cflags Qt5DBus mpv`
LDFLAGS += `$(PKG_CONFIG) --libs Qt5DBus`

SCRIPTS_DIR := $(HOME)/.config/mpv/scripts

PREFIX := /usr/local
PLUGINDIR := $(PREFIX)/lib/mpv-kde-night-color
SYS_SCRIPTS_DIR := /etc/mpv/scripts

.PHONY: \
  install install-user install-system \
  uninstall uninstall-user uninstall-system \
  clean

kde-night-color.so: kde-night-color.c
	$(CC) kde-night-color.c -o kde-night-color.so $(CFLAGS) $(LDFLAGS) -shared -fPIC

ifneq ($(shell id -u),0)
install: install-user
uninstall: uninstall-user
else
install: install-system
uninstall: uninstall-system
endif

install-user: kde-night-color.so
	$(MKDIR) -p $(SCRIPTS_DIR)
	$(INSTALL) -t $(SCRIPTS_DIR) kde-night-color.so

uninstall-user:
	$(RM) -f $(SCRIPTS_DIR)/kde-night-color.so
	$(RMDIR) -p $(SCRIPTS_DIR)

install-system: kde-night-color.so
	$(MKDIR) -p $(DESTDIR)$(PLUGINDIR)
	$(INSTALL) -t $(DESTDIR)$(PLUGINDIR) kde-night-color.so
	$(MKDIR) -p $(DESTDIR)$(SYS_SCRIPTS_DIR)
	$(LN) -s $(PLUGINDIR)/kde-night-color.so $(DESTDIR)$(SYS_SCRIPTS_DIR)

uninstall-system:
	$(RM) -f $(DESTDIR)$(SYS_SCRIPTS_DIR)/kde-night-color.so
	$(RMDIR) -p $(DESTDIR)$(SYS_SCRIPTS_DIR)
	$(RM) -f $(DESTDIR)$(PLUGINDIR)/kde-night-color.so
	$(RMDIR) -p $(DESTDIR)$(PLUGINDIR)

clean:
	rm -f kde-night-color.so
