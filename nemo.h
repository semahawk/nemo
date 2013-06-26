/*
 * nemo.h
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 */

#ifndef NEMO_H
#define NEMO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

/* version of Nemo, obviously */
#define VERSION "0.16.4"

#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#  define BOOL  bool
#  define TRUE  true
#  define FALSE false
#else
#  define BOOL  short
#  define TRUE  1
#  define FALSE 0
#endif

/* order here is quite significant */
#include "config.h"
#include "stack.h"
#include "lexer.h"
#include "object.h"
#include "ast.h"
#include "debug.h"
#include "funcs.h"
#include "scope.h"
#include "error.h"
#include "mem.h"
#include "parser.h"
#include "vars.h"

/*
 * Singly linked list of any dynamically loaded library's handle
 * so that we can dlclose them and not leak any memory
 */
struct LibHandlesList {
  void *handle;
  struct LibHandlesList *next;
};

/*
 * Singly linked list that holds a names of modules/libraries/files
 * (same thing) that were included/used.
 */
struct Included {
  char *name;
  struct Included *next;
};

typedef struct LibHandlesList LibHandlesList;
typedef struct Included Included;

/* TODO: these function names may be confusing */
void Nm_InitModule(NmModuleFuncs *);
BOOL Nm_UseModule(char *name, char *path);
BOOL Nm_IncludeModule(char *name, char *path);
void NmBuiltin_Init(void);
BOOL NmModule_WasIncluded(char *name);

#endif /* NEMO_H */

