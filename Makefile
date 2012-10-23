CC      =  gcc
CFLAGS += -g -W -Wall -std=c99 -march=i686 -O2 -D_POSIX_SOURCE
YACC    =  bison
YFLAGS += -d
LEX     =  flex

OBJECTS = nemo.o nodes_gen.o nodes_exec.o handy.o y.tab.o lex.yy.o

all: nemo clean

nemo: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o nemo

nemo.o: nemo.c nemo.h
	$(CC) $(CFLAGS) -c nemo.c

nodes_gen.o: nodes_gen.c nodes_gen.h
	$(CC) $(CFLAGS) -c nodes_gen.c

nodes_exec.o: nodes_exec.c nodes_exec.h
	$(CC) $(CFLAGS) -c nodes_exec.c

handy.o: handy.c handy.h
	$(CC) $(CFLAGS) -c handy.c

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
