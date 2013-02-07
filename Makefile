CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wunused-variable -std=c99 -O2 -D_POSIX_SOURCE -Iinclude
LEX     = flex
SHELL   = zsh

PREFIX  = /usr/local

OBJECTS = nemo.o gen.o exec.o free.o vars.o cast.o handy.o predef.o userdef.o grammar.o scanner.o

.PHONY: all
all: lemon nemo

lemon: lemon.o
	$(CC) $(CFLAGS) lemon.o -o lemon

lemon.o: lemon.c lempar.c
	$(CC) $(CFLAGS) -c lemon.c

nemo: $(OBJECTS) include/nodes.h
	$(CC) $(OBJECTS) $(CFLAGS) -o nemo

nemo.o: nemo.c include/nemo.h grammar.o scanner.o
	$(CC) $(CFLAGS) -c nemo.c

grammar.o: lemon grammar.y
	./lemon -c -s grammar.y
	$(CC) $(CFLAGS) -c grammar.c

scanner.o: scanner.l
	$(LEX) --header-file=scanner.h scanner.l
	$(CC) $(CFLAGS) -c scanner.c

.PHONY: test install uninstall clean distclean
test: all
	@$(SHELL) t/runner.sh

install: all
	install -D -m 755 nemo $(PREFIX)/bin/nemo

uninstall:
	rm -rf $(PREFIX)/bin/nemo

clean:
	rm -f grammar.c grammar.h grammar.out
	rm -f scanner.c scanner.h
	rm -f *.o

distclean: clean
	rm -f nemo
	rm -f lemon

