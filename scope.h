/*
 *
 * scope.h
 *
 * Created at:  Sat 18 May 2013 11:05:24 CEST 11:05:24
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

#ifndef INTERP_H
#define INTERP_H

#include "nemo.h"
#include "vars.h"

struct Scope {
  /* name of the file, or the strings contents, or stdin, whateva */
  char *name;
  /* list of global variables */
  VariablesList *globals;
  /* list of the functions */
  CFuncsList *cfuncs;
  FuncsList *funcs;
};

/* Doubly linked list of Scope-s */
struct ScopesList {
  struct Scope *scope;
  struct ScopesList *next;
  struct ScopesList *prev;
};

typedef struct Scope Scope;
typedef struct ScopesList ScopesList;

Scope *NmScope_New(char *);
Scope *NmScope_GetCurr(void);
void NmScope_Restore(void);
void NmScope_Tidyup(void);

#endif /* INTERP_H */

