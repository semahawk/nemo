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
 * @name - nm_new_int
 * @desc - create an object that would contain an int
 */
Nob *nm_new_int(int i)
{
  Niob *ob = ncalloc(1, sizeof(Niob));

  ob->type = OT_INTEGER;
  ob->i = i;
  ob->markbit = GC_WHITE;
  ob->fn.print = nm_int_print;

  gc_push(ob);

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

