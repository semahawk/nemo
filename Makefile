CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wall -Wextra -std=c99
SHELL   = sh

LIBPATH = /usr/lib
PREFIX  = /usr/local

OBJECTS  = nemo.o object.o lexer.o parser.o error.o debug.o mem.o ast.o builtin.o scope.o int.o float.o string.o null.o array.o

LIBS = -lreadline

.PHONY: all libnemo libs test install uninstall clean distclean
all: nemo libs

nemo: $(OBJECTS) libnemo
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o nemo

libnemo:
	$(CC) $(CFLAGS) $(OBJECTS) -shared -Wl,-soname,libnemo.so -o libnemo.so

%.o: %.c *.h
	$(CC) $(CFLAGS) $(LIBS) -fPIC -c $< -o $@

libs:
	@cd lib; make

test: nemo
	@$(SHELL) test/runner.sh

install: all
	mkdir -p $(LIBPATH)/nemo
	install -m 644 lib/*.so $(LIBPATH)/nemo
	install -m 644 lib*.so $(LIBPATH)
	install -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(LIBPATH)/libnemo*
	rm -rf $(LIBPATH)/nemo
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f *.o
	rm -f *.so*
	@cd lib; make clean

distclean: clean
	rm -f nemo

