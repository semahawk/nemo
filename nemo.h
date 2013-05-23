/*
 * nemo.h
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 */

#ifndef NEMO_H
#define NEMO_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* version of Nemo, obviously */
#define VERSION "0.15.3"

/* our little own BOOL type */
#define  BOOL short
#ifndef  TRUE
# define TRUE 1
#endif
#ifndef  FALSE
# define FALSE 0
#endif

/* order here is quite significant */
#include "lexer.h"
#include "object.h"
#include "ast.h"
#include "debug.h"
#include "funcs.h"
#include "interp.h"
#include "error.h"
#include "mem.h"
#include "parser.h"
#include "vars.h"

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

void Nm_InitModule(NmModuleFuncs *);
void NmBuiltin_Init(void);

#endif /* NEMO_H */

