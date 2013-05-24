CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wall -Wextra -std=c99
SHELL   = sh

PREFIX  = /usr/local

OBJECTS  = nemo.o object.o lexer.o parser.o error.o debug.o mem.o ast.o
OBJECTS += scope.o builtin.o
OBJECTS += int.o float.o string.o null.o array.o

LIBS = -lreadline

.PHONY: all test install uninstall clean distclean
all: nemo

nemo: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o nemo

nemo.o: nemo.c nemo.h
nemo.h: ast.h debug.h error.h lexer.h parser.h debug.h mem.h object.h
error.o: error.c nemo.h
lexer.o: lexer.c nemo.h
parser.o: parser.c nemo.h
debug.o: debug.c nemo.h
mem.o: mem.c nemo.h
ast.o: ast.c nemo.h
object.o: object.c nemo.h
scope.o: scope.c nemo.h
builtin.o: builtin.c nemo.h

test: nemo
	@$(SHELL) test/runner.sh

install: all
	install -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f *.o

distclean: clean
	rm -f nemo

