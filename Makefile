CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wall -Werror -Wextra -ansi -pedantic -O2

PREFIX  = /usr/local

OBJECTS = nemo.o lexer.o parser.o error.o debug.o

.PHONY: all install uninstall clean distclean
all: nemo

nemo: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o nemo

nemo.o: nemo.c nemo.h
error.o: error.c error.h
lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h
debug.o: debug.c debug.h

install: all
	install -D -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f *.o

distclean: clean
	rm -f nemo

