CC      =  gcc
CFLAGS += -g -W -Wall -std=c99 -O2 -D_POSIX_SOURCE -Iinclude
YACC    =  bison
YFLAGS += -d -v
LEX     =  flex
SHELL   =  zsh

OBJECTS = nemo.o gen.o exec.o free.o vars.o cast.o handy.o predef.o userdef.o y.tab.o lex.yy.o

all: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o bin/nemo

y.tab.o: grammar.y include/nodes.h
	$(YACC) $(YFLAGS) grammar.y
	$(CC) $(CFLAGS) -c y.tab.c

lex.yy.o: scanner.l
	$(LEX) scanner.l
	$(CC) $(CFLAGS) -c lex.yy.c

test:
	@$(SHELL) t/runner.sh

clean:
	rm -f *.tab.c *.tab.h *.yy.c
	rm -f *.o

distclean: clean
	rm -f bin/nemo
