CC = g++
CFLAGS = -W -Wall -O2
LIBS = -lX11 -lXtst
PREFIX = /usr
BINPREFIX = $(PREFIX)/bin

SRC = Space2Ctrl.cpp

all: options s2c

options:
	@echo "Space2Ctrl build options:"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "PREFIX = $(PREFIX)"

s2c: $(SRC) Makefile
	$(CC) -o $@ $(SRC) $(CFLAGS) $(LIBS)

clean:
	@echo "removing Space2Ctrl"
	rm -f Space2Ctrl

install: all
	@echo "installing to $(DESTDIR)$(BINPREFIX)"
	@install -D -m 755 s2c $(DESTDIR)$(BINPREFIX)/s2c
	@install -D -m 755 s2cctl $(DESTDIR)$(BINPREFIX)/s2cctl

uninstall:
	@echo "removing from $(DESTDIR)$(BINPREFIX)"
	@rm -f $(DESTDIR)$(BINPREFIX)/{s2c,s2cctl}

.PHONY: all options clean install uninstall
