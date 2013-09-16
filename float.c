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

Nob *nm_new_float(double f)
{
  ObFreeList *list = nmalloc(sizeof(ObFreeList));
  Nfob *ob = ncalloc(1, sizeof(Nfob));

  ob->type = OT_FLOAT;
  ob->f = f;
  ob->fn.print = nm_float_print;

  /* append to the free_list */
  list->ob = (Nob *)ob;
  list->next = free_list;
  free_list = list;

  return (Nob *)ob;
}

Nob *nm_new_float_from_int(int i)
{
  return nm_new_float((double)i);
}

Nob *nm_float_add(Nob *left, Nob *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) + nm_float_value(right)) == (nm_float_value(left) + nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) + nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) + nm_float_value(right));
  }
}

Nob *nm_float_sub(Nob *left, Nob *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) - nm_float_value(right)) == (nm_float_value(left) - nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) - nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) - nm_float_value(right));
  }
}

Nob *nm_float_mul(Nob *left, Nob *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) * nm_float_value(right)) == (nm_float_value(left) * nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) * nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) * nm_float_value(right));
  }
}

Nob *nm_float_div(Nob *left, Nob *right)
{
  /* check if the result could be represented as an int */
  if ((int)(nm_float_value(left) / nm_float_value(right)) == (nm_float_value(left) / nm_float_value(right))){
    return nm_new_int((int)(nm_float_value(left) / nm_float_value(right)));
  } else {
    return nm_new_float(nm_float_value(left) / nm_float_value(right));
  }
}

CmpRes nm_float_cmp(Nob *left, Nob *right)
{
  return nm_float_value(left) >  nm_float_value(right) ? CMP_GT :
         nm_float_value(left) <  nm_float_value(right) ? CMP_LT :
                                                         CMP_EQ ;
}

Nob *nm_float_incr(Nob *target)
{
  nm_float_value(target) = nm_float_value(target) + 1;

  return target;
}

Nob *nm_float_decr(Nob *target)
{
  nm_float_value(target) = nm_float_value(target) - 1;

  return target;
}

Nob *nm_float_repr(void)
{
  return nm_new_str("float");
}

void nm_float_print(FILE *fp, Nob *ob)
{
  assert(ob->type == OT_FLOAT);

  fprintf(fp, "%g", nm_float_value(ob));
}

void nm_float_destroy(Nob *ob)
{
  assert(ob->type == OT_FLOAT);

  nfree(ob);
}

void nm_float_cleanup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    nm_obj_destroy((Nob *)list->ob);
    nfree(list);
  }
}

