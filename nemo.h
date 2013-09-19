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

#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#else
#  define bool short
#  define true 1
#  define false 0
#endif

/* order here is quite significant */
#include "config.h"
#include "stack.h"
#include "lexer.h"
#include "object.h"
#include "ast.h"
#include "debug.h"
#include "funcs.h"
#include "namespace.h"
#include "error.h"
#include "mem.h"
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
 * (same thing) that were "use"d.
 */
struct Included {
  char *name;
  struct Included *next;
};

typedef struct LibHandlesList LibHandlesList;
typedef struct Included Included;

void nexit();

int name_lookup(char *, Namespace *);

void nm_init_module(NmModuleFuncs *);
bool nm_use_module(char *name);
bool was_module_included(char *name);

Node *nm_parse_file(char *);
Node *nm_parse_string(char *);

void predef_init(void);

#endif /* NEMO_H */

