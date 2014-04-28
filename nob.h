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

enum nob_primitive_type {
  /* that's kind of a draft only */
  OT_INTEGER,
  OT_REAL,
  OT_CHAR,
  OT_STRING,
  OT_TUPLE,
  OT_LIST,
  OT_FUN,

  OT_ANY
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
  char *name;  /* optional name of the type, like "word" */
  enum nob_primitive_type primitive;
  /* additional info about the given type */
  union {
    struct {
      int limitless;
      /* TODO: type of these two should be handled better (probably..) */
      int64_t limit_lower;
      int64_t limit_upper;
    } integer;

    struct {
      /* an array of the struct's/tuple's or unions or whatevers fields */
      /* there probably shouldn't be any limit */
      struct field fields[32];
    } tuple;

    struct {
      /* number of elements (although, I'm not so sure about that...) */
      size_t nmemb;
      /* type of the elements */
      struct nob_type *type;
    } list;

    struct {
      /* the return type, d'oh */
      struct nob_type *return_type;
      /* the parameters the function can take */
      /* as you can see, the limit is 16 */
      struct nob_type *params[16];
      /* the options the function can take */
      /* the limit is, well.. 52, the two alphabets, upper- and lowercase (maybe
       * I'll add numbers later) */
      char *opts;
      /* the function's code */
      struct node *body;
    } func;
  } info;
};

typedef struct nob {
  /* GC mark */
  unsigned char mark;
  /* the object's type, d'oh */
  struct nob_type *type;
  /* pointer to the object's actual value */
  byte_t *ptr;
} Nob;

void types_init(void);
void types_finish(void);
void gc_init(void);
void gc_finish(void);

Nob *new_nob(struct nob_type *type, ...);
struct nob_type *new_type(char *name, enum nob_primitive_type type, ...);
struct nob_type *get_type_by_name(char *name);
size_t sizeof_nob(Nob *ob);
void dump_types(void);
void push_type(struct nob_type *type);
const char *nob_type_to_s(enum nob_primitive_type);

/* make the variables visible */
extern struct nob_type *T_ANY;
extern struct nob_type *T_INT;
extern struct nob_type *T_BYTE;
extern struct nob_type *T_WORD;
extern struct nob_type *T_DWORD;
extern struct nob_type *T_QWORD;
/* lexer, for instance, could use this */
extern struct nob_type **NM_types;
extern struct nob_type **NM_types_curr;

#endif /* NOB_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

