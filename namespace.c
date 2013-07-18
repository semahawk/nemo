/*
 *
 * namespace.c
 *
 * Created at:  Sat 18 May 2013 11:31:05 CEST 11:31:05
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

#include "nemo.h"

static NamespacesList *head = NULL;
static NamespacesList *tail = NULL;
static NamespacesList *curr = NULL;

/* forward */
static void NmNamespace_Destroy(Namespace *);

void NmNamespace_New(char *name)
{
  Namespace *namespace = NmMem_Malloc(sizeof(Namespace));
  NamespacesList *namespace_list = NmMem_Malloc(sizeof(NamespacesList));
  VariablesList *null_list = NmMem_Malloc(sizeof(VariablesList));
  Variable *null = NmMem_Malloc(sizeof(Variable));

  namespace->name = NmMem_Strdup(name);
  namespace->cfuncs = NULL;
  namespace->funcs = NULL;
  namespace->globals = NULL;
  namespace->labels = NULL;

  /* add the "null" variable to the global namespace */
  null->name = NmMem_Strdup("null");
  null->value = NmNull;
  NmVar_SETFLAG(null, NMVAR_FLAG_CONST);
  null_list->var = null;
  null_list->next = namespace->globals;
  namespace->globals = null_list;

  /* append the namespace to the namespaces list */
  namespace_list->namespace = namespace;
  /* the list is empty */
  if (!head && !tail){
    namespace_list->next = head;
    namespace_list->prev = tail;
    head = namespace_list;
    tail = namespace_list;
  }
  /* the list is NOT empty */
  else {
    namespace_list->next = head->next;
    head->next = namespace_list;
    namespace_list->prev = head;
    head = namespace_list;
  }
  /* set the current point to the new one */
  curr = namespace_list;
}

Namespace *NmNamespace_GetCurr(void)
{
  if (!curr){
    NmError_Error("ran out of namespaces!");
    return NULL;
  }

  return curr->namespace;
}

Namespace *NmNamespace_GetByName(char *name)
{
  for (NamespacesList *p = head; p != NULL; p = p->next){
    if (!strcmp(p->namespace->name, name)){
      return p->namespace;
    }
  }

  NmError_SetString("namespace '%s' not found");
  return NULL;
}

void NmNamespace_Restore(void)
{
  printf("restore before: curr is %p, tail is %p\n", (void*)curr, (void*)tail);
  curr = tail;
  printf("restore  after: curr is %p, tail is %p\n", (void*)curr, (void*)tail);
}

/*
 * @name - NmNamespace_Destroy
 * @desc - destroy the current Namespace and all of it's globals and functions
 */
static void NmNamespace_Destroy(Namespace *namespace)
{
  VariablesList *vars;
  VariablesList *vars_next;
  /* destroy all the global variables */
  for (vars = namespace->globals; vars != NULL; vars = vars_next){
    vars_next = vars->next;
    NmMem_Free(vars->var->name);
    NmMem_Free(vars->var);
    NmMem_Free(vars);
  }

  CFuncsList *cfuncs;
  CFuncsList *cfuncs_next;
  /* destroy all the C functions */
  for (cfuncs = namespace->cfuncs; cfuncs != NULL; cfuncs = cfuncs_next){
    cfuncs_next = cfuncs->next;
    NmMem_Free(cfuncs->func->name);
    NmMem_Free(cfuncs->func);
    NmMem_Free(cfuncs);
  }

  LabelsList *labels;
  LabelsList *labels_next;
  /* destroy all the labels */
  for (labels = namespace->labels; labels != NULL; labels = labels_next){
    labels_next = labels->next;
    NmMem_Free(labels->label->name);
    NmMem_Free(labels->label);
    NmMem_Free(labels);
  }

  /* destroy all the elements in the namespaces list */
  NmMem_Free(namespace->name);
  NmMem_Free(namespace);
}

void NmNamespace_Tidyup(void)
{
  NamespacesList *namespaces;
  NamespacesList *next;

  for (namespaces = tail; namespaces != NULL; namespaces = next){
    next = namespaces->next;
    NmNamespace_Destroy(namespaces->namespace);
    NmMem_Free(namespaces);
  }
}

/*
 * Creates a new label, of given <name> and <node>
 * and appends it to the current namespace.
 */
void NmNamespace_NewLabel(char *name, Node *node)
{
  /* create it */
  LabelsList *new_list = NmMem_Malloc(sizeof(LabelsList));
  Label *new_label = NmMem_Malloc(sizeof(Label));
  Namespace *namespace = curr->namespace;

  /* initialize it */
  new_label->name = NmMem_Strdup(name);
  new_label->node = node;
  new_list->label = new_label;

  /* append it */
  new_list->next = namespace->labels;
  namespace->labels = new_list;
}

/*
 * Returns the node associated with a label of a given <name>
 * or NULL if the label was not found.
 */
Node *NmNamespace_GetLabel(char *name)
{
  for (LabelsList *p = curr->namespace->labels; p != NULL; p = p->next)
    if (!strcmp(name, p->label->name))
      return p->label->node;

  return NULL;
}

