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

#include "nemo.h"

/*
 * Yup, a lot of Python inspiration; I'm too dumb to figure it nicely on my own
 */

enum Type {
  OT_INTEGER,
  OT_FLOAT,
  OT_STRING
};

typedef enum Type NmObjectType;
typedef struct Object NmObject;
typedef struct IntObject NmIntObject;
typedef struct FloatObject NmFloatObject;
typedef struct StringObject NmStringObject;

#define NMOBJECT_HEAD                      \
  enum Type type;                          \
  struct {                                 \
    void (*dstr)(Nemo *, NmObject *);      \
    NmObject *(*repr)(Nemo *, NmObject *); \
  } fn;

struct Object {
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

NmObject *NmObject_New(Nemo *NM, const char *s);
NmObject *NmObject_NewFromInt(Nemo *NM, int i);
NmObject *NmObject_NewFromFloat(Nemo *NM, double d);
NmObject *NmObject_NewFromString(Nemo *NM, char *s);

void NmObject_Destroy(Nemo *NM, NmObject *);
void NmObject_DestroyInt(Nemo *NM, NmObject *);
void NmObject_DestroyFloat(Nemo *NM, NmObject *);
void NmObject_DestroyString(Nemo *NM, NmObject *);

void NmObject_Tidyup(Nemo *);

#endif /* OBJECT_H */

