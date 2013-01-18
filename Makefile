CC      =  gcc
CFLAGS += -g -W -Wall -std=c99 -O2 -D_POSIX_SOURCE
YACC    =  bison
YFLAGS += -d
LEX     =  flex
SHELL   =  zsh

OBJECTS = nemo.o gen.o exec.o free.o vars.o cast.o handy.o predef.o y.tab.o lex.yy.o

all: nemo

nemo: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o bin/nemo

nemo.o: nemo.c nemo.h nodes.h
	$(CC) $(CFLAGS) -c nemo.c

nodes_gen.o: gen.c gen.h nodes.h
	$(CC) $(CFLAGS) -c gen.c

nodes_exec.o: exec.c exec.h nodes.h
	$(CC) $(CFLAGS) -c exec.c

nodes_free.o: free.c free.h nodes.h
	$(CC) $(CFLAGS) -c free.c

vars.o: vars.c vars.h nodes.h
	$(CC) $(CFLAGS) -c vars.c

cast.o: cast.c cast.h nodes.h
	$(CC) $(CFLAGS) -c cast.c

handy.o: handy.c handy.h
	$(CC) $(CFLAGS) -c handy.c

predef.o: predef.c predef.h nodes.h
	$(CC) $(CFLAGS) -c predef.c

y.tab.o: grammar.y nodes.h
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
