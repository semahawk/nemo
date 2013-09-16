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
 * @name - nm_new_int
 * @desc - create an object that would contain an int
 */
Nob *nm_new_int(int i)
{
  ObFreeList *list = nmalloc(sizeof(ObFreeList));
  Niob *ob = ncalloc(1, sizeof(Niob));

  ob->type = OT_INTEGER;
  ob->i = i;
  ob->fn.print = nm_int_print;
  ob->fn.binary.add = nm_int_add;
  ob->fn.binary.sub = nm_int_sub;
  ob->fn.binary.mul = nm_int_mul;
  ob->fn.binary.div = nm_int_div;
  ob->fn.binary.mod = nm_int_mod;
  ob->fn.binary.cmp = nm_int_cmp;
  ob->fn.unary.plus = nm_int_plus;
  ob->fn.unary.minus = nm_int_minus;
  ob->fn.unary.negate = nm_int_negate;
  ob->fn.unary.increment = nm_int_incr;
  ob->fn.unary.decrement = nm_int_decr;

  /* append to the free_list */
  list->ob = (Nob *)ob;
  list->next = free_list;
  free_list = list;

  return (Nob *)ob;
}

Nob *nm_new_int_from_void_ptr(void *p)
{
  return nm_new_int((intptr_t)p);
}

Nob *nm_int_add(Nob *left, Nob *right)
{
  return nm_new_int(nm_int_value(left) + nm_int_value(right));
}

Nob *nm_int_sub(Nob *left, Nob *right)
{
  return nm_new_int(nm_int_value(left) - nm_int_value(right));
}

Nob *nm_int_mul(Nob *left, Nob *right)
{
  return nm_new_int(nm_int_value(left) * nm_int_value(right));
}

Nob *nm_int_div(Nob *left, Nob *right)
{
  if (nm_int_value(right) == 0){
    nm_set_error("zero division!");
    return NULL;
  }

  return nm_new_float((float)nm_int_value(left) / (float)nm_int_value(right));
}

Nob *nm_int_mod(Nob *left, Nob *right)
{
  return nm_new_int(nm_int_value(left) % nm_int_value(right));
}

CmpRes nm_int_cmp(Nob *left, Nob *right)
{
  return nm_int_value(left) >  nm_int_value(right) ? CMP_GT :
         nm_int_value(left) <  nm_int_value(right) ? CMP_LT :
                                               CMP_EQ ;
}

Nob *nm_int_plus(Nob *target)
{
  return target;
}

Nob *nm_int_minus(Nob *target)
{
  return nm_new_int(nm_int_value(target) * (-1));
}

Nob *nm_int_negate(Nob *target)
{
  if (nm_obj_boolish(target) == false)
    return nm_new_int(true);
  else
    return nm_new_int(false);
}

Nob *nm_int_incr(Nob *target)
{
  nm_int_value(target) = nm_int_value(target) + 1;

  return target;
}

Nob *nm_int_decr(Nob *target)
{
  nm_int_value(target) = nm_int_value(target) - 1;

  return target;
}

Nob *nm_int_repr(void)
{
  return nm_new_str("int");
}

void nm_int_print(FILE *fp, Nob *ob)
{
  assert(ob->type == OT_INTEGER);

  fprintf(fp, "%d", nm_int_value(ob));
}

void nm_int_destroy(Nob *ob)
{
  assert(ob->type == OT_INTEGER);

  nfree(ob);
}

void nm_int_cleanup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    nm_obj_destroy((Nob *)list->ob);
    nfree(list);
  }
}

