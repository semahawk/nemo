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
 * Type for variables in Nemo
 */
struct Variable {
  /* obviously */
  char *name;
  /* FIXME: needs to be { NmObject * } but when I include "object.h" then they
   * are circulatingly dependant on each other blerh blerh doesn't work */
  void *value;
};

/*
 * Singly linked list for variables
 */
struct VariablesList {
  struct Variable *var;
  struct VariablesList *next;
};

/*
 * The main type for Nemo
 */
struct Nemo {
  /* name of the interpreted file */
  char *source;
  /* The Stack */
  struct {
    /* the stack itself */
    struct Value **it;
    /* 'pointer' to the current element */
    size_t ptr;
    /* size of the stack */
    size_t nmemb;
  } stack;
  /* global variables */
  struct VariablesList *globals;
  /* command line flags */
  struct {
    /* debug (-d?) flags */
    struct {
      BOOL memory;
      BOOL lexer;
      BOOL parser;
      BOOL ast;
    } debug;
  } flags;
};

typedef struct Variable      Variable;
typedef struct VariablesList VariablesList;
typedef struct Nemo          Nemo;

#endif /* NEMO_H */

