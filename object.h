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

typedef enum Type NmObjectType;
typedef struct Object NmObject;
typedef struct NullObject NmNullObject;
typedef struct IntObject NmIntObject;
typedef struct FloatObject NmFloatObject;
typedef struct StringObject NmStringObject;
typedef struct ArrayObject NmArrayObject;
typedef struct FileObject NmFileObject;
typedef struct ObFreeList ObFreeList;

/* result of the *_Cmp function */
typedef enum {
  CMP_EQ,
  CMP_GT,
  CMP_LT
} CmpRes;

typedef NmObject *(*BinaryFunc)(NmObject *, NmObject *);
typedef NmObject *(*UnaryFunc)(NmObject *);

/* forward */
struct Node;

struct Fn {
  void (*dstr)(NmObject *);
  NmObject *(*type_repr)(void);
  void (*print)(FILE *, NmObject *);
  /* binary operations functions */
  struct {
    BinaryFunc add;   /* addition */
    BinaryFunc sub;   /* substraction */
    BinaryFunc mul;   /* multiplication */
    BinaryFunc div;   /* division */
    BinaryFunc mod;   /* modulo */
    BinaryFunc index; /* array/string indexing */
    /* the nm_ast_exec_binop function is taking care to turn
     * this { CmpRes } into { NmObject * } */
    CmpRes (*cmp)(NmObject *, NmObject *); /* compare */
  } binary;
  /* unary operations functions */
  struct {
    UnaryFunc plus;
    UnaryFunc minus;
    UnaryFunc negate;
    UnaryFunc increment;
    UnaryFunc decrement;
  } unary;
};

#define NMOBJECT_HEAD \
  enum Type type;     \
  struct Fn fn;

struct Object {
  NMOBJECT_HEAD
};

struct NullObject {
  NMOBJECT_HEAD
};

struct IntObject {
  NMOBJECT_HEAD
  int i;
};

struct FloatObject {
  NMOBJECT_HEAD
  double f;
};

struct StringObject {
  NMOBJECT_HEAD
  char *s;
};

struct ArrayObject {
  NMOBJECT_HEAD
  size_t nmemb;
  NmObject **a;
};

struct FileObject {
  NMOBJECT_HEAD
  char *name;
  FILE *fp;
};

struct ObFreeList {
  NmObject *ob;
  ObFreeList *next;
};

NmObject *nm_null_repr(void);
void nm_null_print(FILE *, NmObject *);

NmObject *nm_new_obj(const char *);
void nm_obj_destroy(NmObject *);
void nm_obj_cleanup(void);
bool nm_obj_boolish(NmObject *);
NmObject *nm_obj_dup(NmObject *);
NmObject *nm_obj_typetos(NmObjectType);

NmObject *nm_new_int(int);
NmObject *nm_int_add(NmObject *, NmObject *);
NmObject *nm_int_sub(NmObject *, NmObject *);
NmObject *nm_int_mul(NmObject *, NmObject *);
NmObject *nm_int_div(NmObject *, NmObject *);
NmObject *nm_int_mod(NmObject *, NmObject *);
CmpRes    nm_int_cmp(NmObject *, NmObject *);
NmObject *nm_int_plus(NmObject *);
NmObject *nm_int_minus(NmObject *);
NmObject *nm_int_negate(NmObject *);
NmObject *nm_int_incr(NmObject *);
NmObject *nm_int_decr(NmObject *);
NmObject *nm_int_repr(void);
void nm_int_print(FILE *, NmObject *);
void nm_int_destroy(NmObject *);
void nm_int_cleanup(void);
NmObject *nm_new_int_from_void_ptr(void *);
/* a handy macro to simply cast and return the integer value */
#define nm_int_value(ob) (((NmIntObject *)ob)->i)

NmObject *nm_new_float(double);
NmObject *nm_new_float_from_int(int);
NmObject *nm_float_add(NmObject *, NmObject *);
NmObject *nm_float_sub(NmObject *, NmObject *);
NmObject *nm_float_mul(NmObject *, NmObject *);
NmObject *nm_float_div(NmObject *, NmObject *);
CmpRes    nm_float_cmp(NmObject *, NmObject *);
NmObject *nm_float_incr(NmObject *);
NmObject *nm_float_decr(NmObject *);
NmObject *nm_float_repr(void);
void nm_float_print(FILE *, NmObject *);
void nm_float_destroy(NmObject *);
void nm_float_cleanup(void);
/* a handy macro to simply cast and return the float value */
#define nm_float_value(ob) (((NmFloatObject *)ob)->f)

NmObject *nm_new_str(char *);
NmObject *nm_new_str_from_char(char);
NmObject *nm_str_repr(void);
void nm_str_print(FILE *, NmObject *);
NmObject *nm_str_add(NmObject *, NmObject *);
NmObject *nm_str_index(NmObject *, NmObject *);
CmpRes    nm_str_cmp(NmObject *, NmObject *);
void nm_str_destroy(NmObject *);
void nm_str_cleanup(void);
/* a handy macro to simply cast and return the string value */
#define nm_str_value(ob) (((NmStringObject *)ob)->s)

NmObject *nm_file_repr(void);
void nm_file_print(FILE *, NmObject *);

NmObject *nm_new_arr(size_t);
NmObject *nm_new_arr_from_node(struct Node *);
NmObject *nm_arr_repr(void);
void nm_arr_print(FILE *, NmObject *);
NmObject *nm_arr_add(NmObject *, NmObject *);
NmObject *nm_arr_index(NmObject *, NmObject *);
void nm_arr_destroy(NmObject *);
void nm_arr_cleanup(void);
/* a handy macro to simply cast and return the array */
#define nm_arr_value(ob) (((NmArrayObject *)ob)->a)
#define nm_arr_nmemb(ob) (((NmArrayObject *)ob)->nmemb)
/* some handy macros for array elements getting/setting */
#define nm_arr_set_elem(arr,i,v) (nm_arr_value(arr)[i] = v)
#define nm_arr_get_elem(arr,i)   (nm_arr_value(arr)[i])

/*
 * Extern declaration of the "null" which is definied in null.c
 */
extern NmObject *null;

#endif /* OBJECT_H */

