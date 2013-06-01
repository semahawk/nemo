/*
 *
 * vars.c
 *
 * Created at:  Sat 01 Jun 2013 18:08:13 CEST 18:08:13
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

/*
 * Creates a new variable of name <name> and value <value> in the current scope.
 *
 * Note: One have to set flag manually for it (using NmVar_SETFLAG)
 *
 * Returns the variable that was created
 */
Variable *NmVar_New(char *name, NmObject *value)
{
  Scope *scope = NmScope_GetCurr();
  VariablesList *vars_list = NmMem_Malloc(sizeof(VariablesList));
  Variable *var = NmMem_Malloc(sizeof(Variable));

  /* add the newly created variable to the global scope */
  var->name = NmMem_Strdup(name);
  var->value = value;
  vars_list->var = var;
  vars_list->next = scope->globals;
  scope->globals = vars_list;

  return var;
}

