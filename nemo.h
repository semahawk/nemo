/*
 * nemo.h
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 */

#ifndef NEMO_H
#define NEMO_H

/* version of Nemo, obviously */
#define VERSION "0.14.0"

/* our little own BOOL type */
#define  BOOL short
#ifndef  TRUE
# define TRUE 1
#endif
#ifndef  FALSE
# define FALSE 0
#endif

#include "context.h"
#include "ast.h"
#include "debug.h"
#include "error.h"
#include "lexer.h"
#include "mem.h"
#include "object.h"
#include "parser.h"
#include "vars.h"
#include "funcs.h"
#include "interp.h"

/*
 * The main type for Nemo
 */
struct Nemo {
  /* name of the interpreted file */
  char *source;
  /* global variables */
  struct VariablesList *globals;
};

typedef struct Nemo Nemo;

#endif /* NEMO_H */

