CC      =  gcc
CFLAGS += -g -W -Wall -std=c99 -O2 -D_POSIX_SOURCE
YACC    =  bison
YFLAGS += -d
LEX     =  flex

OBJECTS = nemo.o nodes_gen.o nodes_exec.o nodes_free.o vars.o cast.o handy.o predef.o y.tab.o lex.yy.o

all: nemo

nemo: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o bin/nemo

nemo.o: nemo.c nemo.h
	$(CC) $(CFLAGS) -c nemo.c

nodes_gen.o: nodes_gen.c nodes_gen.h
	$(CC) $(CFLAGS) -c nodes_gen.c

nodes_exec.o: nodes_exec.c nodes_exec.h
	$(CC) $(CFLAGS) -c nodes_exec.c

nodes_free.o: nodes_free.c nodes_free.h
	$(CC) $(CFLAGS) -c nodes_free.c

vars.o: vars.c vars.h
	$(CC) $(CFLAGS) -c vars.c

cast.o: cast.c cast.h
	$(CC) $(CFLAGS) -c cast.c

handy.o: handy.c handy.h
	$(CC) $(CFLAGS) -c handy.c

predef.o: predef.c predef.h
	$(CC) $(CFLAGS) -c predef.c

y.tab.o: grammar.y
	$(YACC) $(YFLAGS) grammar.y
	$(CC) $(CFLAGS) -c y.tab.c

lex.yy.o: scanner.l
	$(LEX) scanner.l
	$(CC) $(CFLAGS) -c lex.yy.c

clean:
	rm -f *.tab.c *.tab.h *.yy.c
	rm -f *.o

distclean: clean
	rm -f nemo
