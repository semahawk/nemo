/*
 *
 * scope.c
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

static ScopesList *head = NULL;
static ScopesList *tail = NULL;
static ScopesList *curr = NULL;

/* forward */
static void NmScope_Destroy(Scope *);

void NmScope_New(char *name)
{
  Scope *scope = NmMem_Malloc(sizeof(Scope));
  ScopesList *scope_list = NmMem_Malloc(sizeof(ScopesList));
  VariablesList *null_list = NmMem_Malloc(sizeof(VariablesList));
  Variable *null = NmMem_Malloc(sizeof(Variable));

  scope->name = NmMem_Strdup(name);
  scope->cfuncs = NULL;
  scope->funcs = NULL;
  scope->globals = NULL;
  scope->labels = NULL;
  if (curr)
    scope->parent = curr->scope;
  else
    scope->parent = NULL;

  /* add the "null" variable to the global scope */
  null->name = NmMem_Strdup("null");
  null->value = NmNull;
  NmVar_SETFLAG(null, NMVAR_FLAG_CONST);
  null_list->var = null;
  null_list->next = scope->globals;
  scope->globals = null_list;

  /* append the scope to the scopes list */
  scope_list->scope = scope;
  /* the list is empty */
  if (!head && !tail){
    scope_list->next = head;
    scope_list->prev = tail;
    head = scope_list;
    tail = scope_list;
  }
  /* the list is NOT empty */
  else {
    scope_list->next = head->next;
    head->next = scope_list;
    scope_list->prev = head;
    head = scope_list;
  }
  /* set the current point to the new one */
  curr = scope_list;
}

Scope *NmScope_GetCurr(void)
{
  if (!curr){
    NmError_Error("ran out of scopes!");
    exit(EXIT_FAILURE);
  }

  return curr->scope;
}

void NmScope_Restore(void)
{
  curr = tail;
}

/*
 * @name - NmScope_Destroy
 * @desc - destroy the current Scope and all of it's globals and functions
 */
static void NmScope_Destroy(Scope *scope)
{
  VariablesList *vars;
  VariablesList *vars_next;
  /* destroy all the global variables */
  for (vars = scope->globals; vars != NULL; vars = vars_next){
    vars_next = vars->next;
    NmMem_Free(vars->var->name);
    NmMem_Free(vars->var);
    NmMem_Free(vars);
  }

  CFuncsList *cfuncs;
  CFuncsList *cfuncs_next;
  /* destroy all the C functions */
  for (cfuncs = scope->cfuncs; cfuncs != NULL; cfuncs = cfuncs_next){
    cfuncs_next = cfuncs->next;
    NmMem_Free(cfuncs->func->name);
    NmMem_Free(cfuncs->func);
    NmMem_Free(cfuncs);
  }

  LabelsList *labels;
  LabelsList *labels_next;
  /* destroy all the labels */
  for (labels = scope->labels; labels != NULL; labels = labels_next){
    labels_next = labels->next;
    NmMem_Free(labels->label->name);
    NmMem_Free(labels->label);
    NmMem_Free(labels);
  }

  /* destroy all the elements in the scopes list */
  NmMem_Free(scope->name);
  NmMem_Free(scope);
}

void NmScope_Tidyup(void)
{
  ScopesList *scopes;
  ScopesList *next;

  for (scopes = tail; scopes != NULL; scopes = next){
    next = scopes->next;
    NmScope_Destroy(scopes->scope);
    NmMem_Free(scopes);
  }
}

/*
 * Creates a new label, of given <name> and <node>
 * and appends it to the current scope.
 */
void NmScope_NewLabel(char *name, Node *node)
{
  /* create it */
  LabelsList *new_list = NmMem_Malloc(sizeof(LabelsList));
  Label *new_label = NmMem_Malloc(sizeof(Label));
  Scope *scope = curr->scope;

  /* initialize it */
  new_label->name = NmMem_Strdup(name);
  new_label->node = node;
  new_list->label = new_label;

  /* append it */
  new_list->next = scope->labels;
  scope->labels = new_list;
}

/*
 * Returns the node associated with a label of a given <name>
 * or NULL if the label was not found.
 */
Node *NmScope_GetLabel(char *name)
{
  for (LabelsList *p = curr->scope->labels; p != NULL; p = p->next)
    if (!strcmp(name, p->label->name))
      return p->label->node;

  return NULL;
}

