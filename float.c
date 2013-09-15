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

NmObject *nm_new_float(double f)
{
  ObFreeList *list = nmalloc(sizeof(ObFreeList));
  NmFloatObject *ob = ncalloc(1, sizeof(NmFloatObject));

  ob->type = OT_FLOAT;
  ob->f = f;
  ob->fn.dstr = nm_float_destroy;
  ob->fn.type_repr = nm_float_repr;
  ob->fn.print = nm_float_print;
  ob->fn.binary.add = nm_float_add;
  ob->fn.binary.sub = nm_float_sub;
  ob->fn.binary.mul = nm_float_mul;
  ob->fn.binary.div = nm_float_div;
  ob->fn.binary.index = NULL;
  ob->fn.binary.cmp = nm_float_cmp;
  ob->fn.unary.increment = nm_float_incr;
  ob->fn.unary.decrement = nm_float_decr;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmObject *nm_new_float_from_int(int i)
{
  return nm_new_float((double)i);
}

NmObject *nm_float_add(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) + nm_float_value(right)) == (nm_float_value(left) + nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) + nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) + nm_float_value(right));
  }
}

NmObject *nm_float_sub(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) - nm_float_value(right)) == (nm_float_value(left) - nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) - nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) - nm_float_value(right));
  }
}

NmObject *nm_float_mul(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) * nm_float_value(right)) == (nm_float_value(left) * nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) * nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) * nm_float_value(right));
  }
}

NmObject *nm_float_div(NmObject *left, NmObject *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) / nm_float_value(right)) == (nm_float_value(left) / nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) / nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) / nm_float_value(right));
  }
}

CmpRes nm_float_cmp(NmObject *left, NmObject *right)
{
  return nm_float_value(left) >  nm_float_value(right) ? CMP_GT :
         nm_float_value(left) <  nm_float_value(right) ? CMP_LT :
                                                         CMP_EQ ;
}

NmObject *nm_float_incr(NmObject *target)
{
  nm_float_value(target) = nm_float_value(target) + 1;

  return target;
}

NmObject *nm_float_decr(NmObject *target)
{
  nm_float_value(target) = nm_float_value(target) - 1;

  return target;
}

NmObject *nm_float_repr(void)
{
  return nm_new_str("float");
}

void nm_float_print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  fprintf(fp, "%g", nm_float_value(ob));
}

void nm_float_destroy(NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  nfree(ob);
}

void nm_float_cleanup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    nm_obj_destroy((NmObject *)list->ob);
    nfree(list);
  }
}

