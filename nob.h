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
};

/* forward declaration (`struct field' needs it, but `struct nob_type' needs
 * `struct field') */
struct nob_type;

/* (for now) to be passed from the parser to `new_nob' */
struct field {
  /* the fields name, obviously */
  char *name;
  /* and the type associated with the name */
  struct nob_type *type;
};

struct nob_type {
  char *name; /* optional name of the type, like "word" */
  unsigned size; /* the type's size, in bytes */
  enum nob_primitive_type primitive;
  /* additional info about the given type */
  union {
    struct {
      /* an array of the struct's/tuple's or unions or whatevers fields */
      /* there probably shouldn't be any limit */
      struct field fields[32];
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

    struct {
      /* a type variable */
      char name;
    } variable;
  } info;
};

/* a singly-linked list of <struct nob_type>s */
struct types_list {
  struct nob_type *type;
  struct types_list *next;
};

typedef struct nob {
  /* GC mark */
  unsigned char mark;
  /* the object's type, d'oh */
  struct nob_type *type;
  /* pointer to the object's actual value */
  void *ptr;
} Nob;

/* a singly-linked list of <struct nob>s */
struct nobs_list {
  struct nob *nob;
  struct nobs_list *next;
};

void types_init(void);
void types_finish(void);
void gc_finish(void);

Nob *new_nob(struct nob_type *type, ...);
struct nob_type *new_type(char *name, enum nob_primitive_type type, ...);
struct nob_type *get_type_by_name(char *name);
size_t sizeof_nob(Nob *ob);
void dump_types(void);
void push_type(struct nob_type *type);
const char *nob_type_to_s(enum nob_primitive_type);
bool nob_is_true(Nob *ob);
bool nob_types_are_equal(struct nob_type *, struct nob_type *);
void free_nob(Nob *ob);

/* make the variables visible */
extern struct nob_type *T_INT;
extern struct nob_type *T_INFNUM;
extern struct nob_type *T_BYTE;
extern struct nob_type *T_WORD;
extern struct nob_type *T_DWORD;
extern struct nob_type *T_QWORD;
extern struct nob_type *T_CHAR;
extern struct nob_type *T_REAL;
extern struct nob_type *T_STRING;
/* lexer, for instance, could use this */
extern struct types_list *NM_types;

#endif /* NOB_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

