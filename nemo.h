/*
 * nemo.h
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 */

#ifndef NEMO_H
#define NEMO_H

/* version of Nemo, obviously */
#define VERSION "0.14.0"

#define  BOOL short
#ifndef  TRUE
# define TRUE 1
#endif
#ifndef  FALSE
# define FALSE 0
#endif

/*
 * The main type for Nemo
 */
typedef struct {
  char *source;
  struct {
    struct {
      BOOL memory;
      BOOL lexer;
    } debug;
  } flags;
} Nemo;

char *strdup(Nemo *, char *);

#endif /* NEMO_H */
