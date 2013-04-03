CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Werror -Wextra -ansi -pedantic -O0

PREFIX  = /usr/local

OBJECTS = nemo.o error.o

.PHONY: all
all: nemo

nemo: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o nemo

install: all
	install -D -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f *.o

distclean: clean
	rm -f nemo

