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
  OT_STRING
};

typedef enum Type NmObjectType;
typedef struct Object NmObject;
typedef struct NullObject NmNullObject;
typedef struct IntObject NmIntObject;
typedef struct FloatObject NmFloatObject;
typedef struct StringObject NmStringObject;

struct Fn {
  void (*dstr)(NmObject *);
  NmObject *(*repr)(NmObject *);
  void (*print)(FILE *, NmObject *);
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

void NmNull_Print(FILE *, NmObject *);

NmObject *NmObject_New(const char *);
void NmObject_Destroy(NmObject *);
void NmObject_Tidyup(void);

NmObject *NmObject_NewFromInt(int);
void NmInt_Print(FILE *, NmObject *);
void NmInt_Destroy(NmObject *);
NmObject *NmInt_NewFromVoidPtr(void *);

NmObject *NmObject_NewFromFloat(double);
void NmFloat_Print(FILE *, NmObject *);
void NmFloat_Destroy(NmObject *);

NmObject *NmObject_NewFromString(char *);
void NmString_Print(FILE *, NmObject *);
void NmString_Destroy(NmObject *);

/*
 * Extern declaration of the "null" which is definied in object.c
 */
extern NmObject *NmNull;

#endif /* OBJECT_H */

