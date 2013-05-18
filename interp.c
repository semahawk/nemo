/*
 *
 * interp.c
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

/* Doubly linked list of InterpState-s */
typedef struct InterpStatesList {
  InterpState *is;
  struct InterpStatesList *next;
  struct InterpStatesList *prev;
} InterpStatesList;

static InterpStatesList *head = NULL;
static InterpStatesList *tail = NULL;

InterpState *NmInterpState_New(void)
{
  InterpState *interp = NmMem_Malloc(sizeof(InterpState));
  InterpStatesList *interp_list = NmMem_Malloc(sizeof(InterpStatesList));
  VariablesList *null_list = NmMem_Malloc(sizeof(VariablesList));
  Variable *null = NmMem_Malloc(sizeof(Variable));

  interp->cfuncs = NULL;
  interp->funcs = NULL;
  interp->globals = NULL;

  /* add the "null" variable to the global scope */
  null->name = NmMem_Strdup("null");
  null->value = NmNull;
  NmVar_SETFLAG(null, NMVAR_FLAG_CONST);
  null_list->var = null;
  null_list->next = interp->globals;
  interp->globals = null_list;

  /* append the interp to the interps list */
  interp_list->is = interp;
  /* the list is empty */
  if (!head && !tail){
    interp_list->next = head;
    interp_list->prev = tail;
    head = interp_list;
    tail = interp_list;
  }
  /* the list is NOT empty */
  else {
    interp_list->next = head->next;
    head->next = interp_list;
    interp_list->prev = head;
    head = interp_list;
  }

  return interp;
}

InterpState *NmInterpState_GetCurr(void)
{
  return head->is;
}

/*
 * @name - NmInterpState_Destroy
 * @desc - destroy the current InterpState and all of it's globals and functions
 */
void NmInterpState_Destroy(void)
{
  VariablesList *vars;
  VariablesList *vars_next;
  /* destroy all the global variables */
  for (vars = head->is->globals; vars != NULL; vars = vars_next){
    vars_next = vars->next;
    NmMem_Free(vars->var->name);
    NmMem_Free(vars->var);
    NmMem_Free(vars);
  }

  CFuncsList *cfuncs;
  CFuncsList *cfuncs_next;
  /* destroy all the C functions */
  for (cfuncs = head->is->cfuncs; cfuncs != NULL; cfuncs = cfuncs_next){
    cfuncs_next = cfuncs->next;
    NmMem_Free(cfuncs->func);
    NmMem_Free(cfuncs);
  }

  /* destroy all the elements in the interps list */
  NmMem_Free(head->is->source);
  NmMem_Free(head->is);
  /* and remove it from the interps list */
  /*   if head->prev is NULL, it means it's the only element in the list */
  if (head->prev == NULL){
    NmMem_Free(head);
    head = NULL;
    tail = NULL;
  }
  /*   if head->prev is NOT NULL then there are others */
  else {
    head->prev->next = head->next;
    InterpStatesList *tmp = head->prev;
    NmMem_Free(head);
    head = tmp;
  }
}
