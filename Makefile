CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wall -Wextra -ansi -pedantic

PREFIX  = /usr/local

OBJECTS = nemo.o object.o lexer.o parser.o error.o debug.o mem.o ast.o

LIBS = -lreadline
CFLAGS := $(CFLAGS) $(LIBS)

.PHONY: all install uninstall clean distclean
all: nemo

nemo: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o nemo

nemo.o: nemo.c nemo.h parser.h mem.h debug.h error.h
error.o: error.c error.h
lexer.o: lexer.c lexer.h nemo.h error.h lexer.h debug.h mem.h
parser.o: parser.c parser.h nemo.h error.h lexer.h ast.h debug.h
debug.o: debug.c debug.h nemo.h lexer.h
mem.o: mem.c mem.h nemo.h error.h debug.h
ast.o: ast.c ast.h nemo.h mem.h
object.o: object.c object.h

install: all
	install -D -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f *.o

distclean: clean
	rm -f nemo

