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

/* version of Nemo, obviously */
#define VERSION "0.16.0"

/*
 * Singly linked list of any dynamically loaded library's handle
 * so that we can dlclose them and not leak any memory
 */
struct LibHandlesList {
  void *handle;
  struct LibHandlesList *next;
};

typedef struct LibHandlesList LibHandlesList;

void Nm_InitModule(NmModuleFuncs *);
void Nm_UseModule(char *);
void Nm_IncludeModule(char *);
void NmBuiltin_Init(void);

#endif /* NEMO_H */

