/*
 *
 * funcs.h
 *
 * Created at:  Fri 17 May 2013 20:54:08 CEST 20:54:08
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

#ifndef FUNCS_H
#define FUNCS_H

#include "nemo.h"

/* type of the C functions */
typedef NmObject *(*NmCFunc)(Params *);

struct Func {
  /* name of the function */
  char *name;
  /* it's body */
  Node *body;
};

struct CFunc {
  /* name of the function */
  char *name;
  /* it's body */
  NmCFunc body;
};

/* Simple singly linked list */
struct FuncsList {
  struct Func *func;
  struct FuncsList *next;
};

/* type of the array elements of list of the functions in modules
 * (ouch, lost myself a bit) */
typedef struct ModuleFuncs {
  char *name;
  NmCFunc ptr;
} NmModuleFuncs;

typedef struct FuncsList FuncsList;
typedef struct Func Func;

void Nm_InsertFuncs(NmModuleFuncs *);

#endif /* FUNCS_H */

