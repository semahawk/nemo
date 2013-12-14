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
  OT_INTEGER = 0x00,
  OT_REAL    = 0x01,
  OT_CHAR    = 0x02,
  OT_STRING  = 0x03,
  OT_ARRAY   = 0x04,
  OT_TUPLE   = 0x05,
  OT_FUN     = 0x06
};

struct nob_type {
  char *name;  /* name of the type, like "word" */
  size_t size; /* the type's total size in bytes */
               /* zero means the size is unknown */
  enum nob_primitive_type primitive;
  /* additional info about the given type */
  union {
    struct {
      /* an array of the struct's/tuple's or unions or whatevers fields */
      struct {
        /* the field's name */
        char *name;
        /* and it's type */
        struct nob_type *type;
      } fields[32];
    } mixed;

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
size_t sizeof_nob(Nob *ob);
void dump_types(void);

/* make the variables visible */
extern struct nob_type *T_BYTE;
extern struct nob_type *T_WORD;
extern struct nob_type *T_DWORD;
extern struct nob_type *T_QWORD;

#endif /* NOB_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

