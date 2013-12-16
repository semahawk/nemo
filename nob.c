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
    /* anonymous types don't have a name, so there's no point of freeing it */
    /* I know that free(NULL) is practically a NOP, but, still */
    if (NM_types[i])
      nfree(NM_types[i]->name);
    nfree(NM_types[i]);
  }

  nfree(NM_types);
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
  new.ptr = nmalloc(type->size);

  switch (type->primitive){
    case OT_INTEGER:
    {
      /* {{{ */
      int64_t value = va_arg(vl, int64_t);
      /* check for overflows */
      if (value > 1 << (type->size * 8)){
        fprintf(stderr, "warning: possible overflow! trying to fit %ld in %lu byte%s (%d)\n", value, type->size, type->size == 1 ? "" : "s", 1 << (type->size * 8));
      }

      *new.ptr = value;
      break;
      /* }}} */
    }

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

  va_end(vl);

  return push_nob(&new);
}

/*
 * Return's the size (in bytes) of the given object.
 */
size_t sizeof_nob(Nob *ob)
{
  assert(ob);
  assert(ob->type);

  return ob->type->size;
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

  /* set up the type with some knowns */
  if (name == NULL) /* no name means anonymous type */
    new_type->name = NULL;
  else
    new_type->name = strdup(name);

  new_type->size = 0;
  new_type->primitive = type;

  /* see what the <type> is, so we know how to process the stdargs */
  switch (type){
    case OT_INTEGER:
      new_type->size = va_arg(vl, size_t);
      break;
    case OT_TUPLE: {
      struct field *fields;
      struct field *p;
      unsigned i = 0; /* the counter */
      size_t total_size = 0;

      fields = va_arg(vl, struct field *);
      /* zero-out the tuple's info.mixed */
      memset(new_type->info.mixed.fields, 0, MAX_TUPLE_FIELDS * sizeof(struct field));

      /* calculate the total size */
      for (p = fields; p->type != NULL && p->name != NULL; p++, i++){
        new_type->info.mixed.fields[i].type = p->type;
        /* the parser `strdup's the name, so we don't have to */
        new_type->info.mixed.fields[i].name = p->name;
        /* add to the total size of the whole type */
        total_size += p->type->size;
      }
      /* set the size */
      new_type->size = total_size;

      break;
    }

    /* suspress warnings */
    case OT_REAL:
    case OT_CHAR:
    case OT_STRING:
    case OT_ARRAY:
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

/*
 * DEEEBUG
 */
void dump_types(void)
{
  /* meh, that's quite a mess */
  unsigned i = 0; /* the counter */

  printf("\n## Types Dump:\n");
  for (; i < NM_types_curr - NM_types; i++){
    struct nob_type *type = NM_types[i];
    printf("   %p %s (type: %d, size: %lu)\n", (void *)type, type->name, type->primitive, type->size);
    /* print additional info about tuples */
    if (NM_types[i]->primitive == OT_TUPLE){
      unsigned j = 0; /* additional counter */

      for (; type->info.mixed.fields[j].name != NULL; j++){
        struct field field = type->info.mixed.fields[j];
        printf("     - %s: %p %s (type: %d, size: %lu)\n", field.name, (void *)field.type, field.type->name, field.type->primitive, field.type->size);
      }
    }
  }
  printf("## End\n\n");
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

