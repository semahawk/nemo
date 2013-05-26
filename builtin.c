/*
 *
 * builtin.c
 *
 * Created at:  Fri 17 May 2013 22:26:42 CEST 22:26:42
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

/*
 * "Can't you see
 *  That in the world we live in
 *  Political lies
 *  Are just corporate decisions
 *  They'll take away
 *  All the hopes, not their promises
 *  They'll put an end to this
 *  Land of the livin"
 *
 *  Testament - Souls of Black
 */

#include <stdio.h>
#include <stdlib.h>

#include "nemo.h"

static NmObject *builtin_assert(NmObject *args)
{
  if (NmArray_NMEMB(args) != 2){
    NmError_SetString("wrong number of arguments for function 'assert' (%d when 2 expected)", NmArray_NMEMB(args));
    return NULL;
  }

  NmObject *first  = NmArray_GETELEM(args, 0);
  NmObject *second = NmArray_GETELEM(args, 1);

  /* both are of the same type */
  if (first->type == second->type){
    if (!first->fn.binary.cmp){
      NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
      return NULL;
    }

    if (!NmObject_Boolish(first->fn.binary.cmp(first, second))){
      NmError_SetString("assertion failed");
      return NULL;
    }
  }
  /* they are of different types */
  else {
    /* XXX int and float */
    if (first->type == OT_INTEGER && second->type == OT_FLOAT){
      first = NmFloat_NewFromInt(NmInt_VAL(first));
      if (!first->fn.binary.cmp){
        NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
        return NULL;
      }
    }
    /* XXX float and int */
    else if (first->type == OT_FLOAT && second->type == OT_INTEGER){
      second = NmFloat_NewFromInt(NmInt_VAL(second));
      if (!first->fn.binary.cmp){
        NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
        return NULL;
      }
    }
    /* if anything else, the operation is simply not permitted */
    else {
      NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
      return NULL;
    }
  }

  return NmInt_New(1);
}

static NmObject *builtin_print(NmObject *args)
{
  for (size_t i = 0; i < NmArray_NMEMB(args); i++){
    NmObject_PRINT(stdout, NmArray_GETELEM(args, i));
    /* this the last parameter */
    if (i + 1 == NmArray_NMEMB(args)){
      fprintf(stdout, "\n");
    /* this is NOT the last parameter */
    } else {
      fprintf(stdout, ", ");
    }
  }

  return NmInt_New(1);
}

static NmObject *builtin_id(NmObject *args)
{
  if (NmArray_NMEMB(args) != 1){
    NmError_SetString("wrong number of arguments for function 'assert' (%d when 1 expected)", NmArray_NMEMB(args));
    return NULL;
  }

  return NmInt_NewFromVoidPtr((void *)NmArray_GETELEM(args, 0));
}

static NmModuleFuncs module_funcs[] = {
  { "assert", builtin_assert },
  { "print", builtin_print },
  { "id", builtin_id },
  { NULL, NULL }
};

void NmBuiltin_Init(void)
{
  Nm_InitModule(module_funcs);
}

