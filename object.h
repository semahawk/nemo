/*
 *
 * object.h
 *
 * Created at:  Fri 10 May 2013 18:10:55 CEST 18:10:55
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>

#include "nemo.h"

#define nm_obj_print(fp,ob) ob->fn.print(fp, ob)

/*
 * Yup, a lot of Python inspiration; I'm too dumb to figure it nicely on my own
 */

enum Type {
  OT_NULL      = 1 << 0,
  OT_INTEGER   = 1 << 1,
  OT_FLOAT     = 1 << 2,
  OT_STRING    = 1 << 3,
  OT_ARRAY     = 1 << 4,
  OT_FILE      = 1 << 5,
  /* used only when creating a module in C */
  OT_ANY       = OT_NULL
               | OT_INTEGER
               | OT_FLOAT
               | OT_STRING
               | OT_ARRAY
               | OT_FILE
};

typedef enum Type NobType;
typedef struct ob Nob;
typedef struct null_ob Nnob;
typedef struct int_ob Niob;
typedef struct float_ob Nfob;
typedef struct str_ob Nsob;
typedef struct arr_ob Naob;
typedef struct file_ob Nfhob;
/* we won't need this when we have a garbage collector */
typedef struct ObFreeList ObFreeList;

/* result of the *_Cmp function */
typedef enum {
  CMP_EQ,
  CMP_GT,
  CMP_LT
} CmpRes;

typedef Nob *(*BinaryFunc)(Nob *, Nob *);
typedef Nob *(*UnaryFunc)(Nob *);
typedef CmpRes (*CmpFunc)(Nob *, Nob *);

/* forward, all defined in ast.h */
struct Node;
enum BinaryOp;
enum UnaryOp;

struct Fn {
  void (*print)(FILE *, Nob *);
};

#define NMOBJECT_HEAD \
  enum Type type;     \
  struct Fn fn;

struct ob {
  NMOBJECT_HEAD
};

struct null_ob {
  NMOBJECT_HEAD
};

struct int_ob {
  NMOBJECT_HEAD
  int i;
};

struct float_ob {
  NMOBJECT_HEAD
  double f;
};

struct str_ob {
  NMOBJECT_HEAD
  char *s;
};

struct arr_ob {
  NMOBJECT_HEAD
  size_t nmemb;
  Nob **a;
};

struct file_ob {
  NMOBJECT_HEAD
  char *name;
  FILE *fp;
};

struct ObFreeList {
  Nob *ob;
  ObFreeList *next;
};

Nob *nm_null_repr(void);
void nm_null_print(FILE *, Nob *);

Nob *nm_new_obj(const char *);
void nm_obj_destroy(Nob *);
void nm_obj_cleanup(void);
bool nm_obj_boolish(Nob *);
Nob *nm_obj_dup(Nob *);
Nob *nm_obj_typetos(Nob *);
BinaryFunc nm_obj_has_binary_func(Nob *, enum BinaryOp);
UnaryFunc  nm_obj_has_unary_func(Nob *, enum UnaryOp);
CmpFunc    nm_obj_has_cmp_func(Nob *, enum BinaryOp);

Nob *nm_new_int(int);
Nob *nm_int_add(Nob *, Nob *);
Nob *nm_int_sub(Nob *, Nob *);
Nob *nm_int_mul(Nob *, Nob *);
Nob *nm_int_div(Nob *, Nob *);
Nob *nm_int_mod(Nob *, Nob *);
CmpRes    nm_int_cmp(Nob *, Nob *);
Nob *nm_int_plus(Nob *);
Nob *nm_int_minus(Nob *);
Nob *nm_int_negate(Nob *);
Nob *nm_int_incr(Nob *);
Nob *nm_int_decr(Nob *);
Nob *nm_int_repr(void);
void nm_int_print(FILE *, Nob *);
void nm_int_destroy(Nob *);
void nm_int_cleanup(void);
Nob *nm_new_int_from_void_ptr(void *);
/* a handy macro to simply cast and return the integer value */
#define nm_int_value(ob) (((Niob *)ob)->i)

Nob *nm_new_float(double);
Nob *nm_new_float_from_int(int);
Nob *nm_float_add(Nob *, Nob *);
Nob *nm_float_sub(Nob *, Nob *);
Nob *nm_float_mul(Nob *, Nob *);
Nob *nm_float_div(Nob *, Nob *);
CmpRes    nm_float_cmp(Nob *, Nob *);
Nob *nm_float_incr(Nob *);
Nob *nm_float_decr(Nob *);
Nob *nm_float_repr(void);
void nm_float_print(FILE *, Nob *);
void nm_float_destroy(Nob *);
void nm_float_cleanup(void);
/* a handy macro to simply cast and return the float value */
#define nm_float_value(ob) (((Nfob *)ob)->f)

Nob *nm_new_str(char *);
Nob *nm_new_str_from_char(char);
Nob *nm_str_repr(void);
void nm_str_print(FILE *, Nob *);
Nob *nm_str_add(Nob *, Nob *);
Nob *nm_str_index(Nob *, Nob *);
CmpRes    nm_str_cmp(Nob *, Nob *);
void nm_str_destroy(Nob *);
void nm_str_cleanup(void);
/* a handy macro to simply cast and return the string value */
#define nm_str_value(ob) (((Nsob *)ob)->s)

Nob *nm_file_repr(void);
void nm_file_print(FILE *, Nob *);
void nm_file_destroy(Nob *);

Nob *nm_new_arr(size_t);
Nob *nm_new_arr_from_node(struct Node *);
Nob *nm_arr_repr(void);
void nm_arr_print(FILE *, Nob *);
Nob *nm_arr_add(Nob *, Nob *);
Nob *nm_arr_index(Nob *, Nob *);
void nm_arr_destroy(Nob *);
void nm_arr_cleanup(void);
/* a handy macro to simply cast and return the array */
#define nm_arr_value(ob) (((Naob *)ob)->a)
#define nm_arr_nmemb(ob) (((Naob *)ob)->nmemb)
/* some handy macros for array elements getting/setting */
#define nm_arr_set_elem(arr,i,v) (nm_arr_value(arr)[i] = v)
#define nm_arr_get_elem(arr,i)   (nm_arr_value(arr)[i])

/*
 * Extern declaration of the "null" which is definied in null.c
 */
extern Nob *null;

#endif /* OBJECT_H */

