PKG_CONFIG ?= pkg-config
CXX ?= g++

INSTALL := install
MKDIR := mkdir
RMDIR := rmdir
LN := ln
RM := rm

_PKG_CFLAGS := $(shell $(PKG_CONFIG) --cflags Qt5DBus mpv)
_PKG_LDFLAGS := $(shell $(PKG_CONFIG) --libs Qt5DBus)
CFLAGS += -std=c++11 -Wall -Wextra $(_PKG_CFLAGS)
LDFLAGS += $(_PKG_LDFLAGS)

SCRIPTS_DIR := $(HOME)/.config/mpv/scripts

PREFIX ?= /usr/local
PLUGINDIR := $(PREFIX)/lib/mpv-kde-night-color-playback
SYS_SCRIPTS_DIR := /etc/mpv/scripts

.PHONY: \
  all \
  install install-user install-system \
  uninstall uninstall-user uninstall-system \
  clean \
  compile_commands

SRC_FILE := kde-night-color.c
TARGET_SO := kde-night-color-playback.so

all: $(TARGET_SO)

$(TARGET_SO): $(SRC_FILE)
	$(CXX) $(CFLAGS) -fPIC $(SRC_FILE) -shared -o $(TARGET_SO) $(LDFLAGS)

ifneq ($(shell id -u),0)
install: install-user
uninstall: uninstall-user
else
install: install-system
uninstall: uninstall-system
endif

install-user: kde-night-color-playback.so
	$(MKDIR) -p "$(SCRIPTS_DIR)"
	$(INSTALL) -t "$(SCRIPTS_DIR)" $(TARGET_SO)

uninstall-user:
	$(RM) -f "$(SCRIPTS_DIR)/$(TARGET_SO)"
	-$(RMDIR) -p "$(SCRIPTS_DIR)" 2>/dev/null

install-system: kde-night-color-playback.so
	$(MKDIR) -p "$(DESTDIR)$(PLUGINDIR)"
	$(INSTALL) -t "$(DESTDIR)$(PLUGINDIR)" $(TARGET_SO)
	$(MKDIR) -p "$(DESTDIR)$(SYS_SCRIPTS_DIR)"
	$(LN) -sf "$(PLUGINDIR)/$(TARGET_SO)" "$(DESTDIR)$(SYS_SCRIPTS_DIR)/$(TARGET_SO)"

uninstall-system:
	$(RM) -f "$(DESTDIR)$(SYS_SCRIPTS_DIR)/$(TARGET_SO)"
	-$(RMDIR) -p "$(DESTDIR)$(SYS_SCRIPTS_DIR)" 2>/dev/null
	$(RM) -f "$(DESTDIR)$(PLUGINDIR)/$(TARGET_SO)"
	-$(RMDIR) -p "$(DESTDIR)$(PLUGINDIR)" 2>/dev/null

clean:
	$(RM) -f $(TARGET_SO)

# Target to generate compile_commands.json for VS Code Intellisense
# You need to have 'bear' installed (e.g., sudo apt install bear)
compile_commands:
	bear -- make all
