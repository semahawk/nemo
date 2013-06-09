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

#define NmObject_PRINT(fp,ob) ob->fn.print(fp, ob)

/*
 * Yup, a lot of Python inspiration; I'm too dumb to figure it nicely on my own
 */

enum Type {
  OT_NULL,
  OT_INTEGER,
  OT_FLOAT,
  OT_STRING,
  OT_ARRAY
};

typedef enum Type NmObjectType;
typedef struct Object NmObject;
typedef struct NullObject NmNullObject;
typedef struct IntObject NmIntObject;
typedef struct FloatObject NmFloatObject;
typedef struct StringObject NmStringObject;
typedef struct ArrayObject NmArrayObject;
typedef struct ObFreeList ObFreeList;

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
    BinaryFunc index; /* array/string indexing */
    BinaryFunc cmp;   /* compare */
  } binary;
  /* unary operations functions */
  struct {
    UnaryFunc plus;
    UnaryFunc minus;
    UnaryFunc negate;
  } unary;
};

#define NMOBJECT_HEAD                      \
  enum Type type;                          \
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

struct ObFreeList {
  NmObject *ob;
  ObFreeList *next;
};

NmObject *NmNull_TypeRepr(void);
void NmNull_Print(FILE *, NmObject *);

NmObject *NmObject_New(const char *);
void NmObject_Destroy(NmObject *);
void NmObject_Tidyup(void);
BOOL NmObject_Boolish(NmObject *);

NmObject *NmInt_New(int);
NmObject *NmInt_Add(NmObject *, NmObject *);
NmObject *NmInt_Cmp(NmObject *, NmObject *);
NmObject *NmInt_Plus(NmObject *);
NmObject *NmInt_Minus(NmObject *);
NmObject *NmInt_Negate(NmObject *);
NmObject *NmInt_TypeRepr(void);
void NmInt_Print(FILE *, NmObject *);
void NmInt_Destroy(NmObject *);
void NmInt_Tidyup(void);
NmObject *NmInt_NewFromVoidPtr(void *);
/* a handy macro to simply cast and return the integer value */
#define NmInt_VAL(ob) (((NmIntObject *)ob)->i)

NmObject *NmFloat_New(double);
NmObject *NmFloat_NewFromInt(int);
NmObject *NmFloat_Add(NmObject *, NmObject *);
NmObject *NmFloat_Cmp(NmObject *, NmObject *);
NmObject *NmFloat_TypeRepr(void);
void NmFloat_Print(FILE *, NmObject *);
void NmFloat_Destroy(NmObject *);
void NmFloat_Tidyup(void);
/* a handy macro to simply cast and return the float value */
#define NmFloat_VAL(ob) (((NmFloatObject *)ob)->f)

NmObject *NmString_New(char *);
NmObject *NmString_TypeRepr(void);
void NmString_Print(FILE *, NmObject *);
NmObject *NmString_Index(NmObject *, NmObject *);
void NmString_Destroy(NmObject *);
void NmString_Tidyup(void);
/* a handy macro to simply cast and return the string value */
#define NmString_VAL(ob) (((NmStringObject *)ob)->s)

NmObject *NmArray_New(size_t);
NmObject *NmArray_NewFromNode(struct Node *);
NmObject *NmArray_TypeRepr(void);
void NmArray_Print(FILE *, NmObject *);
NmObject *NmArray_Add(NmObject *, NmObject *);
NmObject *NmArray_Index(NmObject *, NmObject *);
void NmArray_Destroy(NmObject *);
void NmArray_Tidyup(void);
/* a handy macro to simply cast and return the array */
#define NmArray_VAL(ob) (((NmArrayObject *)ob)->a)
#define NmArray_NMEMB(ob) (((NmArrayObject *)ob)->nmemb)
/* some handy macros for array elements getting/setting */
#define NmArray_SETELEM(arr,i,v) (NmArray_VAL(arr)[i] = v)
#define NmArray_GETELEM(arr,i)   (NmArray_VAL(arr)[i])

/*
 * Extern declaration of the "null" which is definied in null.c
 */
extern NmObject *NmNull;

#endif /* OBJECT_H */

