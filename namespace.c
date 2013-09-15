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

/* pointer to the first namespace on the list */
static NamespacesList *head = NULL;
/* pointer to the last namespace on the list */
static NamespacesList *tail = NULL;
/* pointer to the current namespace that is being used */
static NamespacesList *curr = NULL;
/* pointer to the the namespace that was being used before "curr" */
static NamespacesList *prev = NULL;

/* forward */
static void nm_namespace_destroy(Namespace *);

void nm_new_namespace(char *name)
{
  Namespace *namespace = nmalloc(sizeof(Namespace));
  NamespacesList *namespace_list = nmalloc(sizeof(NamespacesList));

  namespace->name = nm_strdup(name);
  namespace->cfuncs = NULL;
  namespace->funcs = NULL;
  namespace->globals = NULL;
  namespace->labels = NULL;

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
  /* update the "prev" */
  prev = curr;
  /* update the "curr" so it points to the new one */
  curr = namespace_list;
  /* create the "__name" variable, which holds the current namespace's name */
  nm_new_var("__name", nm_new_str(name));
}

Namespace *nm_curr_namespace(void)
{
  if (!curr){
    nm_error("ran out of namespaces!");
    return NULL;
  }

  return curr->namespace;
}

Namespace *nm_get_namespace_by_name(char *name)
{
  for (NamespacesList *p = tail; p != NULL; p = p->next)
    if (!strcmp(p->namespace->name, name))
      return p->namespace;

  nm_set_error("namespace '%s' not found");
  return NULL;
}

/*
 * Sets "curr" to the namespace of a given <name>.
 *
 * When the wanted namespace was not found, it simply gets created
 * (nm_new_namespace is kind enough to set "curr" itself).
 */
void nm_switch_namespace(char *name)
{
  for (NamespacesList *p = tail; p != NULL; p = p->next){
    if (!strcmp(p->namespace->name, name)){
      prev = curr;
      curr = p;
      return;
    }
  }

  nm_new_namespace(name);
}

void nm_restore_namespace(void)
{
  NamespacesList *save = curr;
  curr = prev;
  prev = save;
}

/*
 * @name - nm_namespace_destroy
 * @desc - destroy the given <namespace> and all of it's globals and functions
 */
static void nm_namespace_destroy(Namespace *namespace)
{
  VariablesList *vars;
  VariablesList *vars_next;
  /* destroy all the global variables */
  for (vars = namespace->globals; vars != NULL; vars = vars_next){
    vars_next = vars->next;
    nfree(vars->var->name);
    nfree(vars->var);
    nfree(vars);
  }

  CFuncsList *cfuncs;
  CFuncsList *cfuncs_next;
  /* destroy all the C functions */
  for (cfuncs = namespace->cfuncs; cfuncs != NULL; cfuncs = cfuncs_next){
    cfuncs_next = cfuncs->next;
    nfree(cfuncs->func->name);
    nfree(cfuncs->func);
    nfree(cfuncs);
  }

  FuncsList *funcs;
  FuncsList *funcs_next;
  /* destroy all the C functions */
  for (funcs = namespace->funcs; funcs != NULL; funcs = funcs_next){
    funcs_next = funcs->next;
    nfree(funcs->func->name);
    nfree(funcs->func);
    nfree(funcs);
  }

  LabelsList *labels;
  LabelsList *labels_next;
  /* destroy all the labels */
  for (labels = namespace->labels; labels != NULL; labels = labels_next){
    labels_next = labels->next;
    nfree(labels->label->name);
    nfree(labels->label);
    nfree(labels);
  }

  /* destroy all the elements in the namespaces list */
  nfree(namespace->name);
  nfree(namespace);
}

void nm_namespace_cleanup(void)
{
  NamespacesList *namespaces;
  NamespacesList *next;

  for (namespaces = tail; namespaces != NULL; namespaces = next){
    next = namespaces->next;
    nm_namespace_destroy(namespaces->namespace);
    nfree(namespaces);
  }
}

/*
 * Creates a new label, of given <name> and <node>
 * and appends it to the current namespace.
 */
void nm_new_label(char *name, Node *node)
{
  /* create it */
  LabelsList *new_list = nmalloc(sizeof(LabelsList));
  Label *new_label = nmalloc(sizeof(Label));
  Namespace *namespace = curr->namespace;

  /* initialize it */
  new_label->name = nm_strdup(name);
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
Node *nm_get_label(char *name)
{
  for (LabelsList *p = curr->namespace->labels; p != NULL; p = p->next)
    if (!strcmp(name, p->label->name))
      return p->label->node;

  return NULL;
}

#if DEBUG
/*
 * Debug/develop purposes only.
 * Lists the namespaces that were declared.
 */
void nm_list_namespaces(void)
{
  printf("## NAMESPACES:\n");
  for (NamespacesList *p = head; p != NULL; p = p->prev){
    printf("   %p: %s\n", (void *)p->namespace, p->namespace->name);
  }
  printf("##\n");
}
#else
void nm_list_namespaces(void)
{
  /* NOP */;
}
#endif

