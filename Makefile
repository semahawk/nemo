CC      =  gcc
CFLAGS += -g -W -Wunused-variable -std=c99 -O2 -D_POSIX_SOURCE -Iinclude
LEX     =  flex
SHELL   =  zsh

OBJECTS = nemo.o gen.o exec.o free.o vars.o cast.o handy.o predef.o userdef.o grammar.o scanner.o

all: $(OBJECTS) include/nodes.h
	$(CC) $(OBJECTS) $(CFLAGS) -o bin/nemo

nemo.o: nemo.c include/nemo.h grammar.o scanner.o
	$(CC) $(CFLAGS) -c nemo.c

grammar.o: lemon grammar.y
	bin/lemon -c -s grammar.y
	$(CC) $(CFLAGS) -c grammar.c

scanner.o: scanner.l scanner

.PHONY: lemon
lemon: lemon.c lempar.c
	$(CC) $(CFLAGS) -o bin/lemon lemon.c

.PHONY: scanner
scanner: scanner.l
	$(LEX) --header-file=scanner.h scanner.l
	$(CC) $(CFLAGS) -c scanner.c

test:
	@$(SHELL) t/runner.sh

.PHONY: clean distclean
clean:
	rm -f grammar.c grammar.h grammar.out
	rm -f scanner.c scanner.h
	rm -f *.o

distclean: clean
	rm -f bin/nemo
	rm -f bin/lemon

