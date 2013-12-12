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

enum nob_primitive_type {
  /* that's kind of a draft only */
  OT_INTEGER = 1,
  OT_WORD,
  OT_DWORD,
  OT_QWORD,
  OT_REAL,
  OT_CHAR,
  OT_STRING,
  OT_ARRAY,
  OT_TUPLE,
  OT_FUN
};

struct nob_type {
  unsigned id; /* the type's id, to differentiate between them */
  char *name; /* the type's name, eg. "int", "word" */
  size_t size; /* the type's total size in bits, eg. 16 for "word" */
  enum nob_primitive_type primitive;
};

struct nob {
  /* GC mark */
  unsigned char mark;
  /* the object's type */
  struct nob_type type;
  union {
    double r;
    /* to be extended */
  } in;
};

typedef uint64_t Nob;
/* The four most significant bits (the four leftmost) are reserved for internal
 * use. This nibble's value indicates the rest of the integer's
 * interpretation. */
/* Kind of Ruby way, I guess. */
#define NOB_MASK (0xF000000000000000) /* mask for the magic nibble */
/* a macro to retrieve the nibble */
#define NOB_NIBBLE(nob) ((uint8_t)(((nob) & NOB_MASK) >> 60))

/* Here are the different values the magic nibble can be: */
#define NOB_IMMIDIATE_POS ((uint64_t)0xA) /* the rest of the integer (60 bits) is an
immidiate positive value (of an integer) */
#define NOB_IMMIDIATE_NEG ((uint64_t)0xB) /* the rest of the integer (60 bits) is an
immidiate negative value (of an integer) */

#define NOB_IMMIDIATE_VAL(nob) ((signed)(~NOB_MASK & (nob))) /* retrieve the value */
/* TODO: the NOB_IMMIDIATE_VAL macro should take the signedness in account */

/* The immidiate's max value */
#define NOB_IMMIDIATE_MAX ((unsigned)(~NOB_MASK))

/* This method gives us integers in the range:
 *
 *   -1,152,921,504,606,846,975 to 1,152,921,504,606,846,975
 *
 * A bit less (actually, four bits less :P) than if we used 64 bit integers
 * but that way there's no need to allocate the widely used integers */

/* If the magic nibble is different than any of the specified above, then the
 * rest of the integer (60 bits) is treated as a pointer to an
 * allocated `struct nob'. This way, Nemo is capable of addressing up to
 * 1,152,921,504,606,846,975 bits, or 1,073,741,824 GiB, or exactly 1 ????iB */

#define NOB_PTR(nob) ((unsigned char *)(~(NOB_MASK) & (nob)))

/* the functions prototypes */
Nob nob_new_int(int64_t value);

#endif /* NOB_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

