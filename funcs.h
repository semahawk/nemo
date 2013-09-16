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
/*
 * The first parameter is Naob pointer which stores all the arguments.
 *
 * The second parameter is an array of bools which indicate which options
 * are set and which are not.
 *
 * Given an example function initialized as:
 *
 *     { "foobar", foobar, 1, { OT_ANY }, "baz" }
 *
 * we can clearly see that the function "foobar" supports the three
 * options: 'b', 'a' and 'z'. The order here is significant!
 *
 * If this function "foobar" was called like this:
 *
 *     foobar -za "argument";
 *
 * then the appropriate elements in the bool array would be set to true, so,
 * finally, the "opts" argument would look like:
 *
 *     { false, true, true }
 *
 *
 * Then in the function's definition, to test if for example the
 * option 'a' was set, one would write:
 *
 *     if (opts[1]){
 *       // the 'a' option is set
 *     }
 *
 */
typedef Nob *(*NmCFunc)(Nob *args, bool *opts);

struct CFunc {
  /* name of the function */
  char *name;
  /* it's body */
  NmCFunc body;
  /* number of args */
  int argc;
  /* options for the function
   * it's a string so to get the number of the options
   * just do strlen(optv) */
  char *opts;
  /* the types of which the arguments can be */
  /* it's an array of { NobType } */
  NobType *types;
};

struct Func {
  /* name of the function */
  char *name;
  /* it's body */
  Node *body;
  /* number of args */
  unsigned argc;
  /* arguments vector */
  NobType *argv;
  /* options for the function
   * it's a string so to get the number of the options
   * just do strlen(opts) */
  char *opts;
};

/* Simple singly linked list */
struct FuncsList {
  struct Func *func;
  struct FuncsList *next;
};

/* Simple singly linked list */
struct CFuncsList {
  struct CFunc *func;
  struct CFuncsList *next;
};

/* type of the array elements of list of the functions in modules
 * (ouch, lost myself a bit) */
typedef struct ModuleFuncs {
  char *name;
  NmCFunc ptr;
  /* number of arguments it can take */
  int argc;
  /* the types of which the arguments can be */
  /* it's an array of { NobType } */
  NobType types[32];
  /* the names of the options */
  char *opts;
} NmModuleFuncs;

typedef struct CFuncsList CFuncsList;
typedef struct CFunc CFunc;
typedef struct FuncsList FuncsList;
typedef struct Func Func;

void nm_insert_funcs(NmModuleFuncs *);

#endif /* FUNCS_H */

