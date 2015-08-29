/*
 *
 * nob.h
 *
 * Created at:  Sun 24 Nov 15:55:45 2013 15:55:45
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef NOB_H
#define NOB_H

#include <stdint.h>

#include "nemo.h"
#include "utf8.h"

/* object flags (eg. if it's mutable) */
#define NOB_FLAG_MUTABLE (1 << 0)
/* few macros to help with the flags */
#define NOB_FLAG_SET(ob, flag) ((ob) |= (flag))
#define NOB_FLAG_GET(ob) ((ob) & (flag))

/* few handy macros, to help retrieving values from certain kinds of Nob */
/* <ob> is of course of type { struct nob } */
#define NOB_GET_INFNUM(ob) (*(struct infnum *)(ob)->ptr)
#define NOB_GET_INT(ob) ((int)(uintptr_t)(ob)->ptr)
#define NOB_GET_CHAR(ob) ((nchar_t)(uintptr_t)(ob)->ptr)
#define NOB_GET_REAL(ob) (*(double *)(ob)->ptr)

enum nob_primitive_type {
  /* that's kind of a draft only */
  OT_INT,
  OT_REAL,
  OT_CHAR,
  OT_STRING,
  OT_INFNUM,
  OT_TUPLE,
  OT_FUN,
  OT_TYPE_VARIABLE,
  /* custom as in 'user defined' */
  OT_CUSTOM
};

typedef struct nob {
  /* GC mark */
  unsigned char mark;
  /* the object's type, d'oh */
  struct nob_type *type;
  /* pointer to the object's actual value */
  void *ptr;
} Nob;

struct nob_type {
  enum nob_primitive_type primitive;
  /* the type's size, in bytes */
  unsigned size;
  /* the type's optional name */
  char *name;
  /* for easier implementation of the type inference algorithm this field is
   * accessible to every 'type' */
  struct types_list *types;
  /* additional info about the given type */
  union {
    struct {
      /* a type variable */
      char name;
      struct nob_type *instance;
    } var;

    struct {
      char *name;
      /* optional (can be NULL); if so it means the type is generic over the type */
      /* btw, I have *no* idea how to properly name this field */
      /* to make it clear: type 'a list = Nil | Cons of 'a * 'a list */
      /*    here the `var` is the 'a in the ML-like list type definition */
      /* with `var` NULL it's like: type direction = North | West | South | East */
      struct nob_type *var;
    } custom;

    struct {
      struct types_list *elems;
    } tuple;

    struct {
      /* the return type, d'oh */
      struct nob_type *return_type;
      /* the parameters the function can take */
      struct types_list *params;
      /* the options the function can take */
      /* the limit is, well.. 52, the two alphabets, upper- and lowercase (maybe
       * I'll add numbers later) */
      char *opts;
    } func;
  } info;
};

/* a singly-linked list of <struct nob_type>s */
struct types_list {
  struct nob_type *type;
  struct types_list *next;
};

/* a singly-linked list of <struct nob>s */
struct nobs_list {
  struct nob *nob;
  struct nobs_list *next;
};

/*
 * a list of type variables already seen in a scope (usually in a function) to
 * reuse the associated <type> upon seeing a type variable with the same name
 * multiple times
 */
struct type_variables_list {
  nchar_t name;
  struct nob_type *type;
  struct type_variables_list *next;
};

void types_init(void);
void types_finish(void);
void gc_finish(void);

Nob *new_nob(struct nob_type *type, ...);
struct nob_type *new_type(enum nob_primitive_type type, ...);
struct nob_type *get_type_by_name(char *name);
struct types_list *reverse_types_list(struct types_list *list);
unsigned types_list_length(struct types_list *list);
size_t sizeof_nob(Nob *ob);
void dump_types(void);
void push_type(struct nob_type *type);
void nob_print_type(struct nob_type *type);
bool nob_is_true(Nob *ob);
bool is_type_variable(struct nob_type *type);
bool is_type_operator(struct nob_type *type);
bool nob_types_are_equal(struct nob_type *, struct nob_type *);
void free_nob(Nob *ob);

/* defined in nob.c */
extern struct nob_type *T_INT;
extern struct nob_type *T_INFNUM;
extern struct nob_type *T_CHAR;
extern struct nob_type *T_REAL;
extern struct nob_type *T_STRING;
extern struct nob_type *T_LIST;
/* lexer, for instance, could use this */
extern struct types_list *NM_types;

/* defined in nob.c */
extern nchar_t next_type_var_name;

#endif /* NOB_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

