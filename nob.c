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
#include <math.h>

#include "mem.h"
#include "nob.h"
#include "infnum.h"
#include "util.h"
#include "utf8.h"

/* global variables to make the life easier, and not to have to remember the
 * pointer values */
struct nob_type *T_ANY;
struct nob_type *T_INT;
struct nob_type *T_BYTE;
struct nob_type *T_WORD;
struct nob_type *T_DWORD;
struct nob_type *T_QWORD;
struct nob_type *T_CHAR;
struct nob_type *T_REAL;
struct nob_type *T_STRING;

/* the head of a singly-linked list of <struct types_list> */
struct types_list *NM_types;

struct gc_pool {
  /* not a pointer! */
  /* whenever you see a Nob *ob = new_nob(whatever), then it's a pointer to
   * this <nob>'s address */
  Nob nob;
  struct gc_pool *next;
};
/* the garbage collector's object pool */
/* (the head of a singly linked list of <struct gc_pool>s) */
struct gc_pool *NM_gc;

/* {{{ Functions related with types (init, finish, etc) */
void types_init(void)
{
  /* create the standard types */
  T_ANY    = new_type("*",      OT_ANY);
  T_INT    = new_type("int",    OT_INTEGER, 1, 0, 0);
  T_BYTE   = new_type("byte",   OT_INTEGER, 0, (int64_t)CHAR_MIN, CHAR_MAX);
  T_WORD   = new_type("word",   OT_INTEGER, 0, (int64_t)SHRT_MIN, SHRT_MAX);
  T_DWORD  = new_type("dword",  OT_INTEGER, 0, (int64_t)INT_MIN,  INT_MAX);
  T_QWORD  = new_type("qword",  OT_INTEGER, 0, (int64_t)LONG_MIN, LONG_MAX);
  T_CHAR   = new_type("char",   OT_CHAR);
  T_REAL   = new_type("real",   OT_REAL);
  T_STRING = new_type("string", OT_LIST, T_CHAR);
}

void types_finish(void)
{
  unsigned i;
  struct types_list *curr, *next;

  for (curr = NM_types; curr != NULL; curr = next){
    next = curr->next;

    if (!curr->type)
      continue;

    /* anonymous types don't have a name, so there's no point of freeing it */
    /* I know that free(NULL) is practically a NOP, but, still */
    nfree(curr->type->name);

    /* free some additional data associated with the type */
    if (curr->type->primitive == OT_TUPLE){
      /* free the tuple's fields' names */
      for (i = 0; curr->type->info.tuple.fields[i].name != NULL; i++){
        nfree(curr->type->info.tuple.fields[i].name);
      }
    }
    /* free the type itself */
    nfree(curr->type);
  }
}

/*
 * "Pushes" a given <type> to the NM_types array.
 */
void push_type(struct nob_type *type)
{
  struct types_list *new = nmalloc(sizeof(struct types_list));

  new->type = type;
  new->next = NM_types;
  NM_types  = new;
}
/* }}} */
/* {{{ Functions related with Garbage Collector (init, finish, etc) */
void gc_finish(void)
{
  struct gc_pool *curr, *next;

  for (curr = NM_gc; curr != NULL; curr = next){
    next = curr->next;

    /* free the value associated with the object */
    /* unless it's a OT_CHAR (where the pointer is the actual value) */
    if (curr->nob.type->primitive != OT_CHAR)
      nfree(curr->nob.ptr);

    /* free the list's element itself */
    nfree(curr);
  }
}

/*
 * "Pushes" a given <Nob> to the NM_gc pool/array.
 */
static Nob *push_nob(Nob *nob)
{
  struct gc_pool *new = nmalloc(sizeof(struct gc_pool));

  new->nob  = *nob;
  new->next = NM_gc;
  NM_gc     = new;

  return &new->nob;
}
/* }}} */

/* that's a WIP, obviously */
Nob *new_nob(struct nob_type *type, ...)
{
  /* {{{ */
  va_list vl;
  Nob new;

  assert(type);
  va_start(vl, type);

  /* set up the new object with some knowns */
  new.type = type;
  new.ptr = NULL;
  new.mark = 0;

  switch (type->primitive){
    case OT_INTEGER:
    {
      /* {{{ */
      struct infnum value = va_arg(vl, struct infnum);

      new.ptr = nmalloc(sizeof(struct infnum));
      *(struct infnum *)new.ptr = value;
      /* }}} */
      break;
    }
    case OT_CHAR:
    {
      /* {{{ */
      nchar_t value = va_arg(vl, nchar_t);

      /* the pointer is the actual value */
      new.ptr = (void *)(uintptr_t)value;
      /* this approach most likely needs serious help */
      /* }}} */
      break;
    }
    case OT_REAL:
    {
      /* {{{ */
      double value = va_arg(vl, double);

      new.ptr = nmalloc(sizeof(double));
      *(double *)new.ptr = value;
      /* }}} */
      break;
    }
    case OT_LIST:
    {
      /* {{{ */
      /* the list's elements themselves */
      struct nobs_list *elems = va_arg(vl, struct nobs_list *);

      new.ptr = elems;
      /* }}} */
      break;
    }

    /* suspress warnings */
    case OT_STRING:
    case OT_TUPLE:
    case OT_FUN:
      break;
    default:
      break;
  }

  va_end(vl);

  return push_nob(&new);
  /* }}} */
}

/*
 * Returns the `struct nob' type related with the given <name>
 */
struct nob_type *get_type_by_name(char *name)
{
  struct types_list *p;

  for (p = NM_types; p != NULL; p = p->next)
    /* don't compare with anonymous types */
    if (p->type->name)
      if (!strcmp(p->type->name, name))
        return p->type;

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
  /* {{{ */
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
      /* nop */
      break;
    case OT_CHAR:
      /* nop */
      break;
    case OT_REAL:
      /* nop */
      break;
    case OT_INTEGER:
    {
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
    case OT_TUPLE:
    {
      /* {{{ */
      struct field *fields;

      fields = va_arg(vl, struct field *);
      /* zero-out the tuple's info.tuple */
      memset(new_type->info.tuple.fields, 0, MAX_TUPLE_FIELDS * sizeof(struct field));

      memcpy(new_type->info.tuple.fields, fields, MAX_TUPLE_FIELDS * sizeof(struct field));

      assert(fields);
      /* }}} */
      break;
    }
    case OT_LIST:
    {
      /* {{{ */
      struct nob_type *type = va_arg(vl, struct nob_type *);

      new_type->info.list.type = type;
      /* }}} */
      break;
    }
    case OT_FUN:
    {
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
    case OT_STRING:
      break;
    default:
      break;
  }

  /* 'append' the new type to the array */
  push_type(new_type);

  va_end(vl);

  return new_type;
  /* }}} */
}

/*
 * See if a given object is considered to be 'true'
 */
bool nob_is_true(Nob *ob)
{
  assert(ob);

  switch (ob->type->primitive){
    case OT_INTEGER:
      if (infnum_is_zero(NOB_GET_INTEGER(ob)))
        return false;
      else
        return true;

    /* fall through */
    case OT_REAL:
    case OT_CHAR:
    case OT_STRING:
    case OT_TUPLE:
    case OT_LIST:
    case OT_FUN:
    case OT_ANY:
      return false;
  }

  /* should never get here */
  return false;
}

bool nob_types_are_equal(struct nob_type *a, struct nob_type *b)
{
#if DEBUG
  if (a == NULL || b == NULL)
    return false;
#endif

  assert(a != NULL && b != NULL);

  if (a->primitive != b->primitive)
    return false;

  if (a->primitive == OT_LIST){
    /* lists are equal if their underlying types are equal */
    return nob_types_are_equal(a->info.list.type, b->info.list.type);
  } else if (a->primitive == OT_TUPLE){
    /* tuples are equal if each of their respective fields' types are equal */
    unsigned i;

    for (i = 0; i < MAX_TUPLE_FIELDS; i++)
      if (!nob_types_are_equal(a->info.tuple.fields[i].type, b->info.tuple.fields[i].type))
        return false;

    return true;
  } else if (a->primitive == OT_FUN){
    unsigned i;

    if (!nob_types_are_equal(a->info.func.return_type, b->info.func.return_type))
      return false;

    for (i = 0; i < MAX_FUN_PARAMS; i++)
      if (!nob_types_are_equal(a->info.func.params[i], b->info.func.params[i]))
        return false;

    return true;
  } else {
    return true;
  }
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
  struct types_list *p;

  printf("\n## Types Dump:\n\n");
  for (p = NM_types; p != NULL; p = p->next){
    struct nob_type *type = p->type;
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

