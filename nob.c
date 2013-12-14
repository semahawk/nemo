/*
 *
 * nob.c
 *
 * Created at:  Sun 24 Nov 16:28:31 2013 16:28:31
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>

#include "mem.h"
#include "nob.h"
#include "util.h"

/* global variables to make the life easier, and not to have to remember the
 * pointer values */
struct nob_type *T_BYTE  = NULL;
struct nob_type *T_WORD  = NULL;
struct nob_type *T_DWORD = NULL;
struct nob_type *T_QWORD = NULL;

/* an (malloced, d'oh) array of `struct nob_type' pointers */
struct nob_type **NM_types = NULL;
/* pointer to the current type in the array */
struct nob_type **NM_types_curr = NULL;
/* initial size of the array (updated when the array gets expanded) */
size_t NM_types_size = 32;

void types_init(void)
{
  /* set up the array */
  NM_types = ncalloc(NM_types_size, sizeof(struct nob_type));
  NM_types_curr = NM_types;

  /* create the standard types */
  T_BYTE  = new_type("byte",  OT_INTEGER, 1);
  T_WORD  = new_type("word",  OT_INTEGER, 2);
  T_DWORD = new_type("dword", OT_INTEGER, 4);
  T_QWORD = new_type("qword", OT_INTEGER, 8);
}

void types_finish(void)
{
  unsigned i;
  ptrdiff_t offset = NM_types_curr - NM_types;

  for (i = 0; i < offset; i++){
    free(NM_types[i]->name);
    free(NM_types[i]);
  }

  free(NM_types);
}

/*
 * "Pushes" a given <type> to the NM_types array.
 */
static void push_type(struct nob_type *type)
{
  ptrdiff_t offset = NM_types_curr - NM_types;

  /* handle overflow */
  if (offset >= (signed)NM_types_size){
    NM_types_size *= 1.5;
    NM_types = nrealloc(NM_types, sizeof(struct nob_type) * NM_types_size);
    NM_types_curr = NM_types + offset;
  }

  *(NM_types_curr++) = type;
}

/* that's not working, obviously */
Nob *nob_new_int(int64_t value)
{
  Nob *new = nmalloc(sizeof(Nob));

  /*new->type = get_type_by_name("");*/

  return new;
}

/*
 * Return's the size (in bytes) of the given object.
 */
size_t sizeof_nob(Nob *ob)
{
  return 0;
}

/*
 * Creates (and appends to the `types' list) a new type, of a given <name>. The
 * new type is of a primitive type - <type>.
 *
 * The <type> determines how the additional/optional `stdarg' options would be
 * processed/interpreteted.
 */
struct nob_type *new_type(char *name, enum nob_primitive_type type, ...)
{
  /* the new type */
  struct nob_type *new_type = nmalloc(sizeof(struct nob_type));
  /* the stdargs list */
  va_list vl;

  va_start(vl, type);

  /* set up the type with some knowns */
  new_type->name = strdup(name);
  new_type->size = 0;
  new_type->primitive = type;

  /* see what the <type> is, so we know how to process the stdargs */
  switch (type){
    case OT_INTEGER:
      new_type->size = va_arg(vl, size_t);
      break;

    /* suspress warnings */
    case OT_REAL:
    case OT_CHAR:
    case OT_STRING:
    case OT_ARRAY:
    case OT_TUPLE:
    case OT_FUN:
      break;
    default:
      break;
  }

  /* 'append' the new type to the array */
  push_type(new_type);

  va_end(vl);

  return new_type;
}

struct nob_type *get_type(unsigned typeid)
{
  return NULL;
}

char *get_type_name(unsigned typeid)
{
  return NULL;
}

/*
 * DEEEBUG
 */
void dump_types(void)
{
  unsigned i = 0;

  printf("\n## Types Dump:\n");
  for (; i < NM_types_curr - NM_types; i++){
    printf("   %p %s (type: %d, size: %lu)\n", (void *)NM_types[i], NM_types[i]->name, NM_types[i]->primitive, NM_types[i]->size);
  }
  printf("## End\n\n");
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

