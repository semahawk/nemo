CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wall -Wextra -std=c99

PREFIX  = /usr/local

OBJECTS  = nemo.o object.o lexer.o parser.o error.o debug.o mem.o ast.o
OBJECTS += interp.o builtin.o

LIBS = -lreadline

.PHONY: all install uninstall clean distclean
all: nemo

nemo: $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJECTS) -o nemo

nemo.o: nemo.c nemo.h
nemo.h: ast.h debug.h error.h lexer.h parser.h debug.h mem.h object.h
error.o: error.c nemo.h
lexer.o: lexer.c nemo.h
parser.o: parser.c nemo.h
debug.o: debug.c nemo.h
mem.o: mem.c nemo.h
ast.o: ast.c nemo.h
object.o: object.c nemo.h
interp.o: interp.c nemo.h
builtin.o: builtin.c nemo.h

install: all
	install -D -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f *.o

distclean: clean
	rm -f nemo

