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

static InterpState *curr = NULL;

InterpState *NmInterpState_New(void)
{
  InterpState *interp = NmMem_Malloc(sizeof(InterpState));

  interp->funcs = NULL;
  interp->globals = NULL;

  curr = interp;

  return interp;
}

InterpState *NmInterpState_GetCurr(void)
{
  return curr;
}

void NmInterpState_Destroy(InterpState *interp)
{
  VariablesList *vars = NULL;
  VariablesList *next = NULL;
  /* iterate through the global variables */
  for (vars = interp->globals; vars != NULL; vars = next){
    next = vars->next;
    NmMem_Free(vars->var->name);
    NmMem_Free(vars->var);
    NmMem_Free(vars);
  }

  NmMem_Free(interp->source);
  NmMem_Free(interp);
}
