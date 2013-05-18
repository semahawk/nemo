/*
 *
 * vars.h
 *
 * Created at:  Wed 15 May 2013 20:23:34 CEST 20:23:34
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

#ifndef VARS_H
#define VARS_H

#include <stdint.h>

#include "nemo.h"

/* In both the macros <var> is of type { Variable * } */
/* a handy macro to set the given <flag> in the given <var> */
#define NmVar_SETFLAG(var,flag) (var->flags |= 1 << (flag))
/* a handy macro to get the given <flag> from the given <var> */
#define NmVar_GETFLAG(var,flag) (var->flags & (1 << (flag)))

/* these numbers define at which bit the flag is stored */
#define NMVAR_FLAG_CONST 0

/*
 * Type for variables in Nemo
 */
struct Variable {
  /* flags, that describe different behaviours */
  uint8_t flags;
  /* obviously */
  char *name;
  /* the variables value */
  NmObject *value;
};

/*
 * Singly linked list for variables
 */
struct VariablesList {
  struct Variable *var;
  struct VariablesList *next;
};

typedef struct VariablesList VariablesList;
typedef struct Variable      Variable;

#endif /* VARS_H */

