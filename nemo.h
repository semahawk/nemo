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

typedef enum {
  OT_INTEGER,
  OT_FLOAT
} ValueType;

typedef struct {
  ValueType type;
  union {
    int i;
    float f;
  } value;
} Value;

/*
 * The main type for Nemo
 */
typedef struct {
  /* name of the interpreted file */
  char *source;
  /* The Stack */
  struct {
    /* the stack itself */
    Value **it;
    /* 'pointer' to the current element */
    size_t ptr;
    /* size of the stack */
    size_t nmemb;
  } stack;
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
} Nemo;

#endif /* NEMO_H */

