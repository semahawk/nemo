/*
 *
 * float.c
 *
 * Created at:  Sat 18 May 2013 16:29:19 CEST 16:29:19
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

NmObject *NmFloat_New(double f)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmFloatObject *ob = NmMem_Calloc(1, sizeof(NmFloatObject));

  ob->type = OT_FLOAT;
  ob->f = f;
  ob->fn.dstr = NmFloat_Destroy;
  ob->fn.type_repr = NmFloat_TypeRepr;
  ob->fn.print = NmFloat_Print;
  ob->fn.binary.add = NmFloat_Add;
  ob->fn.binary.sub = NmFloat_Sub;
  ob->fn.binary.mul = NmFloat_Mul;
  ob->fn.binary.div = NmFloat_Div;
  ob->fn.binary.index = NULL;
  ob->fn.binary.cmp = NmFloat_Cmp;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmObject *NmFloat_NewFromInt(int i)
{
  return NmFloat_New((double)i);
}

NmObject *NmFloat_Add(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(NmFloat_VAL(left) + NmFloat_VAL(right)) == (NmFloat_VAL(left) + NmFloat_VAL(right))){
    return NmInt_New((int)(NmFloat_VAL(left) + NmFloat_VAL(right)));
  } else {
    return NmFloat_New(NmFloat_VAL(left) + NmFloat_VAL(right));
  }
}

NmObject *NmFloat_Sub(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(NmFloat_VAL(left) - NmFloat_VAL(right)) == (NmFloat_VAL(left) - NmFloat_VAL(right))){
    return NmInt_New((int)(NmFloat_VAL(left) - NmFloat_VAL(right)));
  } else {
    return NmFloat_New(NmFloat_VAL(left) - NmFloat_VAL(right));
  }
}

NmObject *NmFloat_Mul(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(NmFloat_VAL(left) * NmFloat_VAL(right)) == (NmFloat_VAL(left) * NmFloat_VAL(right))){
    return NmInt_New((int)(NmFloat_VAL(left) * NmFloat_VAL(right)));
  } else {
    return NmFloat_New(NmFloat_VAL(left) * NmFloat_VAL(right));
  }
}

NmObject *NmFloat_Div(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(NmFloat_VAL(left) / NmFloat_VAL(right)) == (NmFloat_VAL(left) / NmFloat_VAL(right))){
    return NmInt_New((int)(NmFloat_VAL(left) / NmFloat_VAL(right)));
  } else {
    return NmFloat_New(NmFloat_VAL(left) / NmFloat_VAL(right));
  }
}

CmpRes NmFloat_Cmp(NmObject *left, NmObject *right)
{
  return NmFloat_VAL(left) >  NmFloat_VAL(right) ? CMP_GT :
         NmFloat_VAL(left) <  NmFloat_VAL(right) ? CMP_LT :
                                                   CMP_EQ ;
}

NmObject *NmFloat_TypeRepr(void)
{
  return NmString_New("float");
}

void NmFloat_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  fprintf(fp, "%g", NmFloat_VAL(ob));
}

void NmFloat_Destroy(NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  NmMem_Free(ob);
}

void NmFloat_Tidyup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy((NmObject *)list->ob);
    NmMem_Free(list);
  }
}

