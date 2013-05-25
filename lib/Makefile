CC      = gcc
CFLAGS := $(CFLAGS) -g -W -Wall -Wextra -std=c99 -I..
SHELL   = sh

OBJECTS = Math.o

LIBS = -lnemo -lm

.PHONY: all clean
all: libs

libs: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(LIBS) -fPIC -c $< -o $@
	$(CC) $(CFLAGS) -L.. $(LIBS) -shared -Wl,-soname,$(patsubst %.c,%.so,$(wildcard *.c)) -o $(patsubst %.c,%.so,$(wildcard *.c)) $@

clean:
	rm -rf *.o
	rm -rf *.so

