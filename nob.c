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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "mem.h"
#include "nob.h"
#include "util.h"

/* global variables to make the life easier, and not to have to remember the
 * pointer values */
struct nob_type *T_ANY   = NULL;
struct nob_type *T_INT   = NULL;
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

/* the garbage collector object pool */
/* (an array of Nobs (NOT Nob pointers!)) */
Nob *NM_gc = NULL;
/* the current 'slot' in the pool */
Nob *NM_gc_curr = NULL;
/* the size of the pool */
size_t NM_gc_size = 32;

/* {{{ Functions related with types (init, finish, etc) */
void types_init(void)
{
  /* set up the array */
  NM_types = ncalloc(NM_types_size, sizeof(struct nob_type *));
  NM_types_curr = NM_types;

  /* create the standard types */
  T_ANY   = new_type("*",   OT_ANY);
  T_INT   = new_type("int",   OT_INTEGER, 1, 0, 0);
  T_BYTE  = new_type("byte",  OT_INTEGER, 0, (int64_t)CHAR_MIN, CHAR_MAX);
  T_WORD  = new_type("word",  OT_INTEGER, 0, (int64_t)SHRT_MIN, SHRT_MAX);
  T_DWORD = new_type("dword", OT_INTEGER, 0, (int64_t)INT_MIN,  INT_MAX);
  T_QWORD = new_type("qword", OT_INTEGER, 0, (int64_t)LONG_MIN, LONG_MAX);
}

void types_finish(void)
{
  unsigned i, j;
  ptrdiff_t offset = NM_types_curr - NM_types;

  for (i = 0; i < offset; i++){
    /* anonymous types don't have a name, so there's no point of freeing it */
    /* I know that free(NULL) is practically a NOP, but, still */
    if (NM_types[i])
      nfree(NM_types[i]->name);
    /* free some additional data associated with the type */
    if (NM_types[i]->primitive == OT_TUPLE){
      /* free the tuple's fields' names */
      for (j = 0; NM_types[i]->info.tuple.fields[j].name != NULL; j++){
        nfree(NM_types[i]->info.tuple.fields[j].name);
      }
    }
    /* free the type itself */
    nfree(NM_types[i]);
  }

  nfree(NM_types);
}

/*
 * "Pushes" a given <type> to the NM_types array.
 */
void push_type(struct nob_type *type)
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
/* }}} */
/* {{{ Functions related with Garbage Collector (init, finish, etc) */
void gc_init(void)
{
  /* set up the pool */
  NM_gc = ncalloc(NM_gc_size, sizeof(Nob));
  NM_gc_curr = NM_gc;
}

void gc_finish(void)
{
  /* because it's an array of Nobs (not Nob pointers), we only need to free the
   * array, and not the objects themselves as well */
  Nob *p = NM_gc;

  for (; p != NM_gc_curr; p++){
    /* free the value associated with the object */
    nfree(p->ptr);
  }

  /* free the whole pool */
  nfree(NM_gc);
}

/*
 * "Pushes" a given <Nob> to the NM_gc pool/array.
 */
static Nob *push_nob(Nob *nob)
{
  Nob *save = NM_gc_curr;
  ptrdiff_t offset = NM_gc_curr - NM_gc;

  /* handle overflow */
  if (offset >= (signed)NM_gc_size){
    NM_gc_size *= 1.5;
    NM_gc = nrealloc(NM_gc, sizeof(struct nob_type) * NM_gc_size);
    NM_gc_curr = NM_gc + offset;
  }

  *(NM_gc_curr++) = *nob;

  return save;
}
/* }}} */

/* that's a WIP, obviously */
Nob *new_nob(struct nob_type *type, ...)
{
  va_list vl;
  Nob new;

  va_start(vl, type);

  /* set up the new object with some knowns */
  new.type = type;
  new.ptr = NULL; /* FIXME */

  switch (type->primitive){
    case OT_INTEGER:
    {
      /* {{{ */
      int64_t value = va_arg(vl, int64_t);
      /* TODO: set it's value */
      break;
      /* }}} */
    }

    /* suspress warnings */
    case OT_REAL:
    case OT_CHAR:
    case OT_STRING:
    case OT_TUPLE:
    case OT_FUN:
      break;
    default:
      break;
  }

  va_end(vl);

  return push_nob(&new);
}

/*
 * Returns the `struct nob' type related with the given <name>
 */
struct nob_type *get_type_by_name(char *name)
{
  unsigned i = 0;

  for (; i < NM_types_curr - NM_types; i++)
    /* don't compare with anonymous types */
    if (NM_types[i]->name)
      if (!strcmp(NM_types[i]->name, name))
        return NM_types[i];

  return NULL;
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

  /* zero-out the whole type */
  memset(new_type, 0x0, sizeof(struct nob_type));

  /* set up the type with some knowns */
  if (name == NULL) /* no name means anonymous type */
    new_type->name = NULL;
  else
    new_type->name = strdup(name);

  new_type->primitive = type;

  /* see what the <type> is, so we know how to process the stdargs */
  switch (type){
    case OT_ANY:
      /* nothing */
      break;
    case OT_INTEGER: {
      /* {{{ */
      int64_t limitless   = va_arg(vl, int64_t);
      int64_t limit_lower = va_arg(vl, int64_t);
      int64_t limit_upper = va_arg(vl, int64_t);

      new_type->info.integer.limitless = limitless; /* no limits by default */
      new_type->info.integer.limit_lower = limit_lower;
      new_type->info.integer.limit_upper = limit_upper;
      /* }}} */
      break;
    }
    case OT_TUPLE: {
      /* {{{ */
      struct field *fields;

      fields = va_arg(vl, struct field *);
      /* zero-out the tuple's info.tuple */
      memset(new_type->info.tuple.fields, 0, MAX_TUPLE_FIELDS * sizeof(struct field));

      assert(fields);
      /* }}} */
      break;
    }
    case OT_LIST: {
      /* {{{ */
      struct nob_type *type = va_arg(vl, struct nob_type *);

      new_type->info.list.type = type;
      /* }}} */
      break;
    }
    case OT_FUN: {
      /* {{{ */
      struct nob_type *return_type = va_arg(vl, struct nob_type *);
      struct nob_type **params = va_arg(vl, struct nob_type **);
      struct nob_type **p;
      unsigned i = 0;

      /* if params is NULL then that means the function doesn't take any
       * parameters */
      if (params != NULL){
        for (p = params; *p != NULL; p++){
          new_type->info.func.params[i++] = *p;
        }
      } else {
        /*new_type->info.func.params = (struct nob_type **)NULL;*/
      }

      new_type->info.func.return_type = return_type;
      /* }}} */
      break;
    }
    /* suspress warnings */
    case OT_REAL:
    case OT_CHAR:
    case OT_STRING:
      break;
    default:
      break;
  }

  /* 'append' the new type to the array */
  push_type(new_type);

  va_end(vl);

  return new_type;
}

/*
 * Return the primitive type as a string.
 */
const char *nob_type_to_s(enum nob_primitive_type type)
{
  switch (type){
    case OT_INTEGER: return "integer";
    case OT_REAL:    return "real";
    case OT_CHAR:    return "char";
    case OT_STRING:  return "string";
    case OT_TUPLE:   return "tuple";
    case OT_LIST:    return "list";
    case OT_FUN:     return "function";
    case OT_ANY:     return "any (*)";
  }

  return "##unknown_type##nob_type_to_s##";
}

/*
 * DEEEBUG
 */
void dump_types(void)
{
  /* meh, that's quite a mess */
  unsigned i = 0; /* the counter */

  printf("\n## Types Dump:\n\n");
  for (; i < NM_types_curr - NM_types; i++){
    struct nob_type *type = NM_types[i];
    printf("   %p", (void *)type);
    if (type->name != NULL)
      printf(" \"%s\"", type->name);
    printf("\n");
    printf("   - type: %s\n", nob_type_to_s(type->primitive));

    /* print additional info about some certain types */
    if (type->primitive == OT_INTEGER){
      /* {{{ */
      if (!type->info.integer.limitless){
        printf("   - lim %ld, %ld\n", type->info.integer.limit_lower, type->info.integer.limit_upper);
      } else {
        printf("   - limitless\n");
      }
      /* }}} */
    } else if (type->primitive == OT_TUPLE){
      /* {{{ */
      unsigned j = 0; /* additional counter */

      if (type->info.tuple.fields != NULL){
        printf("   - fields:\n");
        for (; type->info.tuple.fields[j].name != NULL; j++){
          struct field field = type->info.tuple.fields[j];
          printf("     + %s: %p", field.name, (void *)field.type);
          if (field.type->name != NULL)
            printf(" \"%s\"", field.type->name);
          printf("\n");
        }
      }
      /* }}} */
    } else if (type->primitive == OT_LIST){
      /* {{{ */
      struct nob_type *t = type->info.list.type;

      printf("   - type %p", (void *)t);
      if (t == NULL){
        printf(" polymorphic ");
      } else {
        if (t->name != NULL)
          printf(" \"%s\"", t->name);
        printf("\n");
      }
      /* }}} */
    } else if (type->primitive == OT_FUN){
      /* {{{ */
      struct nob_type *ret = type->info.func.return_type;
      struct nob_type **params = type->info.func.params;
      struct nob_type **p;

      printf("   - return type:\n");
      if (ret == NULL){
        printf("     = *\n");
      } else {
        printf("     = %p", (void *)ret);
        if (ret->name != NULL)
          printf(" \"%s\"", ret->name);
        printf("\n");
      }

      if (params != NULL && *params != NULL){
        printf("   - parameters:\n");
        for (p = params; *p != NULL; p++){
          printf("     + %p", (void *)(*p));
          if ((*p)->name != NULL)
            printf(" \"%s\"", (*p)->name);
          printf("\n");
        }
      } else {
        printf("     . no parameters\n");
      }
      /* }}} */
    }
    printf("\n");
  }
  printf("## End\n\n");
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

