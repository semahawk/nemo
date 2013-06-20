/*
 *
 * int.c
 *
 * Created at:  Sat 18 May 2013 16:27:42 CEST 16:27:42
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
 * Simple singly linked list that contains any object that was allocated and
 * needs to be freed.
 */
static ObFreeList *free_list = NULL;

/*
 * @name - NmObject_NewFromInt
 * @desc - create an object that would contain an int
 */
NmObject *NmInt_New(int i)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmIntObject *ob = NmMem_Calloc(1, sizeof(NmIntObject));

  ob->type = OT_INTEGER;
  ob->i = i;
  ob->fn.dstr = NmInt_Destroy;
  ob->fn.type_repr = NmInt_TypeRepr;
  ob->fn.print = NmInt_Print;
  ob->fn.binary.add = NmInt_Add;
  ob->fn.binary.sub = NmInt_Sub;
  ob->fn.binary.mul = NmInt_Mul;
  ob->fn.binary.div = NmInt_Div;
  ob->fn.binary.mod = NmInt_Mod;
  ob->fn.binary.cmp = NmInt_Cmp;
  ob->fn.unary.plus = NmInt_Plus;
  ob->fn.unary.minus = NmInt_Minus;
  ob->fn.unary.negate = NmInt_Negate;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmObject *NmInt_NewFromVoidPtr(void *p)
{
  return NmInt_New((intptr_t)p);
}

NmObject *NmInt_Add(NmObject *left, NmObject *right)
{
  return NmInt_New(NmInt_VAL(left) + NmInt_VAL(right));
}

NmObject *NmInt_Sub(NmObject *left, NmObject *right)
{
  return NmInt_New(NmInt_VAL(left) - NmInt_VAL(right));
}

NmObject *NmInt_Mul(NmObject *left, NmObject *right)
{
  return NmInt_New(NmInt_VAL(left) * NmInt_VAL(right));
}

NmObject *NmInt_Div(NmObject *left, NmObject *right)
{
  if (NmInt_VAL(right) == 0){
    NmError_SetString("zero division!");
    return NULL;
  }

  return NmFloat_New((float)NmInt_VAL(left) / (float)NmInt_VAL(right));
}

NmObject *NmInt_Mod(NmObject *left, NmObject *right)
{
  return NmInt_New(NmInt_VAL(left) % NmInt_VAL(right));
}

CmpRes NmInt_Cmp(NmObject *left, NmObject *right)
{
  return NmInt_VAL(left) >  NmInt_VAL(right) ? CMP_GT :
         NmInt_VAL(left) <  NmInt_VAL(right) ? CMP_LT :
                                               CMP_EQ ;
}

NmObject *NmInt_Plus(NmObject *target)
{
  return target;
}

NmObject *NmInt_Minus(NmObject *target)
{
  return NmInt_New(NmInt_VAL(target) * (-1));
}

NmObject *NmInt_Negate(NmObject *target)
{
  if (NmObject_Boolish(target) == FALSE)
    return NmInt_New(TRUE);
  else
    return NmInt_New(FALSE);
}

NmObject *NmInt_TypeRepr(void)
{
  return NmString_New("int");
}

void NmInt_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_INTEGER);

  fprintf(fp, "%d", NmInt_VAL(ob));
}

void NmInt_Destroy(NmObject *ob)
{
  assert(ob->type == OT_INTEGER);

  NmMem_Free(ob);
}

void NmInt_Tidyup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy((NmObject *)list->ob);
    NmMem_Free(list);
  }
}

