/*
 *
 * namespace.h
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

/*
 * Example label:
 *
 *   bye: print "Goobye!";
 *
 * translates to this structure as follows:
 *
 *   name = "bye";
 *   node = 0x68124c0 (that's just random)
 *
 */
struct Label {
  char *name;
  Node *node;
};

/*
 * A simple singly linked list of labels
 */
struct LabelsList {
  struct Label *label;
  struct LabelsList *next;
};

struct Namespace {
  /* name of namespace, eg. main, Math */
  char *name;
  /* list of global variables */
  VariablesList *globals;
  /* list of the functions */
  CFuncsList *cfuncs;
  FuncsList *funcs;
  /* list of labels in the namespace */
  struct LabelsList *labels;
  /* a pointer to the parent namespace */
  struct Namespace *parent;
};

/* Doubly linked list of Namespace-s */
struct NamespacesList {
  struct Namespace *namespace;
  struct NamespacesList *next;
  struct NamespacesList *prev;
};

typedef struct Label Label;
typedef struct LabelsList LabelsList;
typedef struct Namespace Namespace;
typedef struct NamespacesList NamespacesList;

Namespace *NmNamespace_GetCurr(void);
Namespace *NmNamespace_GetByName(char *);
void NmNamespace_New(char *name);
void NmNamespace_Restore(void);
void NmNamespace_Tidyup(void);
void NmNamespace_NewLabel(char *name, Node *node);
Node *NmNamespace_GetLabel(char *name);

#endif /* INTERP_H */

